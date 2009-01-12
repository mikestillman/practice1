--		Copyright 1994 by Daniel R. Grayson

-- a flavor of input stream where line numbers and
-- column numbers are maintained properly

-- Also, if a \ comes at the end of the line, then it swallows the next
-- line up to a nonspace

-- Also, these are *text* files, so we translate all three flavors of line termination
-- into a single \n.

use C;
use system;
use strings;
use stdio;
use ctype;

export Position := {filename:string, line:ushort, column:ushort, loadDepth:uchar};
export (s:Position) === (t:Position) : bool := s == t || s.filename === t.filename && s.line == t.line && s.column == t.column;
export dummyPosition := Position("{*dummy file name*}",ushort(0),ushort(0),uchar(loadDepth));
shorten(s:string):string := (				    -- purely textual
     -- shorten filenames like "/a/b/c/../d////e/f" to "/a/b/d/e/f"
     while true do (
	  i := 0;
	  j := 0;
	  while (
	       if i == length(s) then return s;
	       if i+3 < length(s) && s.i == '/' && s.(i+1) == '.' && s.(i+2) == '.' && s.(i+3) == '/'
	       then (
		    s = substr(s,0,j) + substr(s,i+3);
		    false
		    )
	       else if i+2 < length(s) && s.i == '/' && s.(i+1) == '.' && s.(i+2) == '/'
	       then (
		    s = substr(s,0,i) + substr(s,i+2);
		    false
		    )
	       else if i+2 == length(s) && s.i == '/' && s.(i+1) == '.'
	       then (
		    s = substr(s,0,i);
		    false
		    )
	       else if i+1 < length(s) && s.i == '/' && s.(i+1) == '/'
	       then (
		    s = substr(s,0,i) + substr(s,i+1);
		    false
		    )
	       else true
	       )
	  do (
	       if s.i == '/' then j=i;
	       i = i+1;
	       );
	  );
     );
isAbsolutePath(s:string):bool := (                         			    -- purely textual
     -- eventually make this happen only in MSDOS
     length(s) >= 1 && s.0 == '/' ||
     length(s) >= 3 && s.1 == ':' && s.2 == '/' ||
     s === "stdio"
     );
absoluteFilename(filename:string):string := (					    -- purely textual
     if !isAbsolutePath(filename)
     then filename = getcwd() + filename;		    -- using getcwd() instead of getenv("PWD"), which is what emacs thinks about
     shorten(filename)
     -- if we wanted to take real paths into account we would do it like this instead:
     -- f := realpath(filename);
     -- if length(filename) == 0 || filename.(length(filename)-1) == '/' && length(f)>0 && f.(length(f)-1) != '/' then f = f + '/';
     -- f
     );
export relativizeFilename(cwd:string,filename:string):string := (
     cwd = absoluteFilename(cwd);
     if length(cwd) == 0 || cwd.(length(cwd)-1) != '/' then cwd = cwd + '/';
     filename = absoluteFilename(filename);
     i := 0;
     while i < length(cwd) && i < length(filename) && cwd.i == filename.i do i = i+1;
     if i < length(cwd) && i == length(filename) && cwd.i == '/' then i = i+1;
     while i > 0 && cwd.(i-1) != '/' do i = i-1;
     filename = substr(filename,i);
     while i < length(cwd)
     do (
	  if cwd.i == '/' then filename = "../" + filename; 
	  i = i+1);
     filename);
export relativizeFilename(filename:string):string := relativizeFilename(getcwd(),filename);
export minimizeFilename(filename:string):string := (
     a := relativizeFilename(filename);
     b := absoluteFilename(filename);
     c := if length(a) <= length(b) then a else b;
     if length(c) == 0 then c = "./";
     c);
export verifyMinimizeFilename(filename:string):string := (
     filename = expandFileName(filename);
     p := when realpath(filename) is null do return filename is s:string do s;
     f := minimizeFilename(filename);
     if isAbsolutePath(f) then return f;
     if !(length(f) >= 3 && f.0 == '.' && f.1 == '.' && f.2 == '/') then (
	  -- we assume realpath(getenv("PWD")) and getcwd() would be the same, so the only problem for
	  -- emacs would be if the minimized path begins with ../
	  return f;
	  );
     pwd := getenv("PWD")+"/";
     if pwd === getcwd() then return f;
     h := when
     realpath(getcwd()+f)			       -- this is correct
     is null do return filename is s:string do s;
     k := when
     realpath(shorten(pwd+f))                    -- this simulates what emacs does, textually, which might be wrong
     is null do return filename is s:string do s;
     if k === h then f else h);
export tostring(w:Position) : string := (
     if w == dummyPosition 
     then "{*dummy position*}"
     else errfmt(verifyMinimizeFilename(w.filename),int(w.line),int(w.column + 1),int(w.loadDepth)));
export (o:file) << (w:Position) : file := (
     o << tostring(w)
     );
export SuppressErrors := false;
cleanscreen():void := (
     flush(stdout);
     if stdout.outfd == stderr.outfd && !atEndOfLine(stdout) then (
	  stdout << '\n';
     	  flush(stdout);
	  )
     );

export recursionDepth := 0;
export recursionLimit := 300;

printMessage(position:Position,message:string):void := (
     if !SuppressErrors then (
     	  cleanscreen();
     	  stderr << position << '[' << recursionDepth << "]: " << message << endl;
	  );
     );
export printErrorMessage(position:Position,message:string):void := (
     printMessage(position, if message.0 == '-' then message else "error: "+message)
     );
export printWarningMessage(position:Position,message:string):void := printMessage(position,"warning: "+message);
export printErrorMessage(filename:string,line:ushort,column:ushort,message:string):void := (
     printErrorMessage(Position(filename,line,column,uchar(0)), message);
     );
export (o:file) << (p:(null or Position)):file := (
     when p is null do o is w:Position do o << w
     );
export copy(p:Position):Position := Position(
     p.filename, p.line, p.column, uchar(loadDepth));
export PosFile := {file:file, lastchar:int, pos:Position};
export dummyPosFile := PosFile(dummyfile,0,dummyPosition);
export fileError(f:PosFile):bool := fileError(f.file);
export fileErrorMessage(f:PosFile):string := fileErrorMessage(f.file);
export makePosFile(o:file):PosFile := PosFile(o, 0,
     Position(o.filename, ushort(1), ushort(0), uchar(loadDepth)));
export peek(o:PosFile, offset:int):int := (
     i := 0;
     prevchar := o.lastchar;
     c := 0;
     while (
	  c = peek(o.file,i);
	  prevchar = c;
	  i < offset
	  )
     do (
	  i = i+1;
	  );
     c);
export peek(o:PosFile):int := peek(o,0);
export isatty(o:PosFile):bool := o.file.inisatty;
export close(o:PosFile):int := (
     when close(o.file) is errmsg do ERROR else 0
     );
export setprompt(o:PosFile,prompt:function():string):void := setprompt(o.file,prompt);
export unsetprompt(o:PosFile):void := unsetprompt(o.file);
export openPosIn(filename:string):(PosFile or errmsg) := (
     when openIn(filename)
     is f:file do (PosFile or errmsg)(
	  PosFile(f,0,Position(
		    if isAbsolutePath(f.filename)
		    then f.filename
		    else getcwd() + f.filename,
		    ushort(1),ushort(0),uchar(loadDepth))))
     is s:errmsg do (PosFile or errmsg)(s)
     );
roundup(n:uchar,d:int):uchar := uchar(((int(n)+d-1)/d)*d);
tabwidth := 8;
export getc(o:PosFile):int := (
     prevchar := o.lastchar;
     c := getc(o.file);
     if c == ERROR || c == EOF then return c;
     o.lastchar = c;
     if c == int('\n') then (
	  o.pos.line = o.pos.line + 1;
	  o.pos.column = ushort(0);
	  )
     else if c == int('\t') then (
	  o.pos.column = ushort(((int(o.pos.column)+8)/8)*8);
	  )
     else (
	  o.pos.column = o.pos.column + 1;
	  );
     c );
export flush(o:PosFile):void := flushinput(o.file);

-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/d "
-- End:
