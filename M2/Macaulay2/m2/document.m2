--		Copyright 1994-2002 by Daniel R. Grayson

debugDoc = () -> commandInterpreter local symbol

maximumCodeWidth := 120

DocDatabase = null

local exampleBaseFilename
local exampleOutputFilename
local currentNodeName

duplicateDocWarning := () -> error ("warning: documentation already provided for '", currentNodeName, "'")

-----------------------------------------------------------------------------
-- sublists, might be worthy making public
-----------------------------------------------------------------------------
sublists := (x,f,g,h) -> (
     -- x is a list with elements i
     -- apply g to those i for which f i is true
     -- apply h to the sublists, possibly empty, including those at the beginning and end, of elements between the ones for which f i is true
     -- return the results in the same order
     p := positions(x, f);
     mingle(
	  apply( prepend(-1,p), append(p,#x), (i,j) -> h take(x,{i+1,j-1})),
	  apply( p, i -> g x#i)))

-----------------------------------------------------------------------------
-- unformatting document tags
-----------------------------------------------------------------------------
-- we need to be able to do this only for the document tags we have shown to the user in formatted form 
unformatTag := new MutableHashTable
record      := f -> x -> (
     val := f x; 
     if val =!= x then unformatTag#val = x; 
     val)

-----------------------------------------------------------------------------
-- normalizing document tags
-----------------------------------------------------------------------------
   -- The normalized form for simple objects will be the symbol whose value is the object
   -- (We don't document objects that are not stored in global variables.)
   -- This allows us to write documentation links like
   --                TO "sin" 
   -- or
   --	             TO sin
   -- or
   --	             TO symbol sin
   -- and have them all get recorded the same way
normalizeDocumentTag           = method(SingleArgumentDispatch => true)
normalizeDocumentTag   String := key -> if isGlobalSymbol key then getGlobalSymbol key else key
normalizeDocumentTag   Symbol := identity
normalizeDocumentTag Sequence := identity
normalizeDocumentTag  Nothing := key -> symbol null
normalizeDocumentTag    Thing := key -> (
     if key.?Symbol and value key.Symbol === key then return key.Symbol;
     if ReverseDictionary#?key then return ReverseDictionary#key;
     error("encountered unidentifiable document tag: ",key);
     )

isDocumentableThing = x -> null =!= package x		    -- maybe not quite right

packageTag = method(SingleArgumentDispatch => true)	    -- assumes the input key has been normalized
packageTag   Symbol := key -> package key
packageTag   String := key -> currentPackage
packageTag  Package := identity
packageTag Sequence := key -> youngest \\ package \ key
packageTag    Thing := key -> ( p := package key; if p === null then currentPackage else p)

isDocumentableTag = method(SingleArgumentDispatch => true)  -- assumes the input key has been normalized
isDocumentableTag   Symbol := s -> null =!= packageTag s
isDocumentableTag   String := s -> true
isDocumentableTag Sequence := s -> all(s, isDocumentableThing)
isDocumentableTag    Thing := s -> false

-----------------------------------------------------------------------------
-- formatting document tags
-----------------------------------------------------------------------------
   -- The formatted form should be a human-readable string, and different normalized tags should yield different formatted tags.
   -- The formatted tag is used for two purposes:
   --    for display in menus and links
   --    as the key for access in a database, where the key must be a string

Strings := hashTable { Sequence => "(...)", List => "{...}", Array => "[...]" }
toStr := s -> if Strings#?s then Strings#s else toString s
formatDocumentTag           = method(SingleArgumentDispatch => true)
	  
alphabet := set characters "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'"

formatDocumentTag Thing    := toString
formatDocumentTag String   := s -> s

fSeqInitialize := (toString,toStr) -> new HashTable from {
     (4,NewOfFromMethod) => s -> ("new ", toString s#1, " of ", toString s#2, " from ", toStr s#3),
     (4,cohomology,ZZ  ) => s -> ("HH_", toStr s#1, "^", toStr s#2, " ", toStr s#3),
     (4,homology,ZZ    ) => s -> ("HH^", toStr s#1, "_", toStr s#2, " ", toStr s#3),
     (3,NewFromMethod  ) => s -> ("new ", toString s#1, " from ", toStr s#2),
     (3,NewOfMethod    ) => s -> ("new ", toString s#1, " of ", toString s#2),
     (3,symbol " "     ) => s -> (toStr s#1, " ", toStr s#2),
     (3,homology,ZZ    ) => s -> ("HH_", toStr s#1, " ", toStr s#2),
     (3,cohomology,ZZ  ) => s -> ("HH^", toStr s#1, " ", toStr s#2),
     (2,homology       ) => s -> ("HH ", toStr s#1),
     (2,cohomology     ) => s -> ("HH ", toStr s#1),
     (2,NewMethod      ) => s -> ("new ", toString s#1),
     (2,symbol ~       ) => s -> (toStr s#1, " ", toStr s#0), -- postfix operator
     (2,symbol !       ) => s -> (toStr s#1, " ", toStr s#0), -- postfix operator
     (3,class,Symbol   ) => s -> (toStr s#1, " ", toString s#0, " ", toStr s#2),-- infix operator
     (2,class,Symbol   ) => s -> (toString s#0, " ", toStr s#1),-- prefix operator
     (2,class,ScriptedFunctor,ZZ) => s -> (
	  hh := s#0;
	  if hh.?subscript and hh.?superscript then toString s
	  else if hh.?subscript   then (toString s#0, " _ ", toStr s#1)
	  else if hh.?superscript then (toString s#0, " ^ ", toStr s#1)
	  else (toString s#0, " ", toStr s#1)),
     (3,class,ScriptedFunctor,ZZ) => s -> (
	  if s#0 .? subscript
	  then (toString s#0, "_", toStr s#1, "(", toStr s#2, ")")
	  else (toString s#0, "^", toStr s#1, "(", toStr s#2, ")")),
     (4,class,ScriptedFunctor,ZZ) => s -> (
	  if s#0 .? subscript
	  then (toString s#0, "_", toStr s#1, "(", toStr s#2, ",", toStr s#3, ")")
	  else (toString s#0, "^", toStr s#1, "(", toStr s#2, ",", toStr s#3, ")")),
     4 => s -> (toString s#0, "(", toStr s#1, ",", toStr s#2, ",", toStr s#3, ")"),
     3 => s -> (toString s#0, "(", toStr s#1, ",", toStr s#2, ")"),
     2 => s -> (toString s#0, " ", toStr s#1),
     (Symbol,5)=>s -> ( toString s#0, "(", toStr s#1, ",", toStr s#2, ",", toStr s#3, ",", toString s#-1, "=>...)" ),
     (Symbol,4)=>s -> ( toString s#0, "(", toStr s#1, ",", toStr s#2, ",", toString s#-1, "=>...)" ),
     (Symbol,3)=>s -> ( toString s#0, "(", toStr s#1, ",", toString s#-1, "=>...)" ),
     (Symbol,2)=>s -> ( toString s#0, "(..., ", toString s#-1, "=>...)" )
     }

fSeq := fSeqInitialize(toString,toStr)
formatDocumentTag Sequence := record(
     s -> concatenate (
	  if #s == 0                                            then toString
	  else if             fSeq#?(#s,s#0)                    then fSeq#(#s,s#0)
	  else if #s >= 1 and fSeq#?(#s,s#0,s#1)                then fSeq#(#s,s#0,s#1)
	  else if #s >= 1 and fSeq#?(#s, class, class s#0, s#1) then fSeq#(#s, class, class s#0, s#1)
	  else if             fSeq#?(#s, class, class s#0)      then fSeq#(#s, class, class s#0)
	  else if             fSeq#?(class s#-1,#s)             then fSeq#(class s#-1,#s)
	  else if             fSeq#?#s                          then fSeq#(#s)
								else toString) s)

fSeqTO := fSeqInitialize(i -> TO i, i -> TO i)
formatDocumentTagTO := method(SingleArgumentDispatch => true)
formatDocumentTagTO Thing := x -> TT formatDocumentTag x
formatDocumentTagTO Sequence := (
     s -> SEQ toList (
	  if #s == 0                                              then toString
	  else if             fSeqTO#?(#s,s#0)                    then fSeqTO#(#s,s#0)
	  else if #s >= 1 and fSeqTO#?(#s,s#0,s#1)                then fSeqTO#(#s,s#0,s#1)
	  else if #s >= 1 and fSeqTO#?(#s, class, class s#0, s#1) then fSeqTO#(#s, class, class s#0, s#1)
	  else if             fSeqTO#?(#s, class, class s#0)      then fSeqTO#(#s, class, class s#0)
	  else if             fSeqTO#?#s                          then fSeqTO#(#s)
	                                                          else toString) s)

-----------------------------------------------------------------------------
-- verifying the tags
-----------------------------------------------------------------------------
-- here we check that the method a putative document tag documents is actually installed
verifyTag := method(SingleArgumentDispatch => true)
verifyTag Thing    := s -> null
verifyTag Sequence := s -> (
     if s#?-1 and class s#-1 === Symbol then (		    -- e.g., (res,Strategy) or (res,Module,Strategy)
	  fn := drop(s,-1);
	  opt := s#-1;
	  if #fn === 1 then (
	       fn = fn#0;
	       if not instance(fn, Function) then error "expected first element of document tag for optional argument to be a function";
	       )
	  else (
	       if not instance(lookup fn, Function) then error("no method installed for document tag '", formatDocumentTag fn, "'");
	       fn = fn#0;
	       );
	  if not (options fn)#?opt then error("expected ", opt, " to be an option of ", fn))
     else (						    -- e.g., (res,Module) or (symbol **, Module, Module)
	  if class lookup s =!= Function then error("documentation provided for '", formatDocumentTag s, "' but no method installed")))
verifyTag Option   := s -> error "old style option documentation tag"

-----------------------------------------------------------------------------
-- making document tags
-----------------------------------------------------------------------------
-- We need three bits of information about a document tag:
--     the original key	    	    e.g., (operator **,Module,Module)
--     the formatted key            e.g., "Module ** Module"
--     the title of the package     e.g., "Main"
-- Here we assemble them together.
DocumentTag = new Type of BasicList			    -- {normalized key (symbol or sequence or string), formatted key (string), package title (string)}
net DocumentTag := x -> x#1
makeDocumentTag = key -> (
     verifyTag key;
     key = normalizeDocumentTag key;
     fkey := formatDocumentTag key;
     pkg := packageTag key;
     new DocumentTag from {key,fkey,pkg})
-----------------------------------------------------------------------------
-- fixing up hypertext
-----------------------------------------------------------------------------

trimline0 := x -> selectRegexp ( "^(.*[^ ]|) *$",1, x)
trimline  := x -> selectRegexp ( "^ *(.*[^ ]|) *$",1, x)
trimline1 := x -> selectRegexp ( "^ *(.*)$",1, x)
addspaces := x -> if x#?0 then if x#-1=="." then concatenate(x,"  ") else concatenate(x," ") else x

fixup := method(SingleArgumentDispatch => true)
flat := method()
flat Thing := identity
flat SEQ := x -> toSequence x
flat Nothing := x -> ()
fixflat := z -> splice apply(z, i -> flat fixup i)
fixup Thing      := z -> error("unrecognizable item inside documentation: ", toString z)
fixup Nothing    := identity				       -- null
fixup Sequence   := z -> SEQ fixflat z
fixup List       := z -> SEQ fixflat z
fixup MarkUpList := z -> apply(z,fixup)			       -- recursion
fixup Option     := z -> z#0 => fixup z#1		       -- Headline => "...", ...
fixup UL         := z -> apply(z, i -> fixup if class i === TO then TOH i#0 else i)
fixup PRE        := identity
fixup CODE       := identity
fixup TO         := x -> TO if x#?1 then { normalizeDocumentTag x#0, concatenate drop(toSequence x,1) } else { normalizeDocumentTag x#0 }
fixup TO2        := x -> TO2{ normalizeDocumentTag x#0, concatenate drop(toSequence x,1) }
fixup TOH        := x -> TOH{ normalizeDocumentTag x#0 }
fixup MarkUpType := z -> z{}				       -- convert PARA to PARA{}
fixup Function   := z -> z				       -- allow Function => f 
fixup String     := s -> (				       -- remove clumsy newlines within strings
     ln := lines s;
     if not ln#?1 then return s;
     concatenate ({addspaces trimline0 ln#0}, addspaces \ trimline \take(ln,{1,#ln-2}), {trimline1 ln#-1}))

new Hypertext from List := (h,x) -> splice apply(x, i -> flat i)
hypertext = x -> Hypertext fixup x

-----------------------------------------------------------------------------
-- installing the documentation
-----------------------------------------------------------------------------

Nothing << Thing := (x,y) -> null			    -- turning off the output is easy to do
DocumentableValueType := set { 
     Boolean, 
     HashTable, 
     Function, 
     BasicList, 
     Nothing,
     File
     }
UndocumentableValue := hashTable { symbol environment => true, symbol commandLine => true }
documentableValue := key -> (
     class key === Symbol and value key =!= key
     and not UndocumentableValue#?key and DocumentableValueType#?(basictype value key))

---- how could this have worked (so soon)?
-- scan(flatten(pairs \ globalDictionaries), (name,sym) -> if documentableValue sym then Symbols#(value sym) = sym)


file := null

-----------------------------------------------------------------------------
-- getting database records
-----------------------------------------------------------------------------

extractBody := x -> if x.?Description then x.Description
getRecord := (pkg,key) -> pkg#"documentation"#key	    -- for Databases, insert 'value' here
getPackage := key -> scan(
     value \ values PackageDictionary,
     pkg -> (
	  d := pkg#"documentation";
	  if d#?key then break pkg))
getDoc := key -> (
     fkey := formatDocumentTag key;
     pkg := getPackage fkey;
     if pkg =!= null then getRecord(pkg,fkey))
if debugLevel > 10 then getDoc = on (getDoc, Name => "getDoc")
getOption := (key,tag) -> (
     s := getDoc key;
     if s =!= null and s#?tag then s#tag)
getBody := key -> getOption(key,Description)
-----------------------------------------------------------------------------
-- process examples
-----------------------------------------------------------------------------

extractExamplesLoop            := method(SingleArgumentDispatch => true)
extractExamplesLoop Thing      := x -> {}
extractExamplesLoop EXAMPLE    := toList
extractExamplesLoop MarkUpList := x -> join apply(toSequence x, extractExamplesLoop)

extractExamples := (docBody) -> (
     examples := extractExamplesLoop docBody;
     if #examples > 0 then currentPackage#"example inputs"#currentNodeName = examples;
     docBody)

M2outputRE := "(\n\n)i+[1-9][0-9]* : "
M2outputREindex := 1
separateM2output = method()
separateM2output String := r -> (
     while r#?0 and r#0 == "\n" do r = substring(1,r);
     while r#?-1 and r#-1 == "\n" do r = substring(0,#r-1,r);
     separateRegexp(M2outputRE,M2outputREindex,r))

getFileName := body -> (
     x := select(1, body, i -> class i === Option and #i === 2 and first i === FileName);
     if #x > 0 then x#0#1 else null
     )

makeFileName := (key,filename,pkg) -> (			 -- may return 'null'
     if pkg#?"package prefix" and pkg#"package prefix" =!= null 
     then pkg#"package prefix" | LAYOUT#"packageexamples" pkg#"title" | if filename =!= null then filename else toFilename key
     )

exampleResultsFound := false
exampleResults := {}
exampleCounter := 0
checkForExampleOutputFile := (node,pkg) -> (
     exampleCounter = 0;
     exampleResults = {};
     exampleResultsFound = false;
     exampleOutputFilename = null;
     if debugLevel > 1 then stderr << "exampleBaseFilename = " << exampleBaseFilename << endl;
     if exampleBaseFilename =!= null then (
	  exampleOutputFilename = exampleBaseFilename | ".out";
	  if debugLevel > 0 then (
	       if debugLevel > 1 then stderr << "checking for example results in file '" << exampleOutputFilename << "' : " << (if fileExists exampleOutputFilename then "it exists" else "it doesn't exist") << endl;
	       );
	  if fileExists exampleOutputFilename then (
	       -- read, separate, and store example results
	       exampleResults = pkg#"example results"#node = drop(separateM2output get exampleOutputFilename,-1);
	       if debugLevel > 1 then stderr << "node " << node << " : " << boxList \\ net \ exampleResults << endl;
	       exampleResultsFound = true)))
processExample := x -> (
     a :=
     if exampleResultsFound and exampleResults#?exampleCounter
     then {x, CODE exampleResults#exampleCounter}
     else (
	  if exampleResultsFound and #exampleResults === exampleCounter then (
	       stderr << "warning : example results file " << exampleOutputFilename << " terminates prematurely" << endl;
	       );
	  {x, CODE concatenate("i", toString (exampleCounter+1), " : ",x)}
	  );
     exampleCounter = exampleCounter + 1;
     a)
processExamplesLoop := s -> (
     if class s === EXAMPLE then ExampleTABLE apply(select(toList s, i -> i =!= null), processExample)
     else if class s === Sequence or instance(s,MarkUpList)
     then apply(s,processExamplesLoop)
     else s)
processExamples := (pkg,fkey,docBody) -> (
     exampleBaseFilename = makeFileName(fkey,getFileName docBody,pkg);
     checkForExampleOutputFile(fkey,pkg);
     processExamplesLoop docBody)

-----------------------------------------------------------------------------
-- 'document' function
-----------------------------------------------------------------------------

nonNull := x -> select(x,t->t=!=null)
fixupList := x -> apply(nonNull x,fixup)
enlist := x -> if class x === List then x else {x}
fixupTable := new HashTable from {
     Key => identity,
     FormattedKey => identity,
     Usage => fixup,
     Function => fixup,
     Inputs => fixupList,
     Outputs => fixupList,
     Results => fixupList,
     OldSynopsis => identity,				    -- old
     FileName => identity,
     Headline => identity,
     Description => extractExamples @@ hypertext,
     Caveat => v -> if v =!= null then fixup SEQ { PARA BOLD "Caveat", SEQ v },
     SeeAlso => v -> if v =!= {} and v =!= null then fixup SEQ { PARA BOLD "See also", UL (TO \ enlist v) },
     Subnodes => v -> MENU apply(nonNull enlist v, x -> (
	       if class x === TO then x
	       else if class x === TOH then TO x#0
	       else if class x === String then x
	       else error ("unrecognizable Subnode list item: ",x," in node ",fkey)))
     }
caveat := key -> getOption(key,Caveat)
seealso := key -> getOption(key,SeeAlso)
theMenu := key -> getOption(key,Subnodes)
documentOptions := new HashTable from {
     Key => true,
     FormattedKey => true,
     Usage => true,
     Function => true,
     Inputs => true,
     Outputs => true,
     Results => true,
     OldSynopsis => true,				    -- old
     FileName => true,
     Headline => true,
     Description => true,				    -- the "body"
     SeeAlso => true,
     Caveat => true,
     Subnodes => true }
document = method( SingleArgumentDispatch => true )

document List := z -> document toSequence z
document Thing := z -> document singleton z
document Sequence := args -> (
     opts := new MutableHashTable;
     scan(args, arg -> if class arg === Option then (
	       key := arg#0;
	       val := arg#1;
	       if not documentOptions#key then error("unknown option ",key);
	       if opts#?key then error("option ",key," encountered twice");
	       opts#key = val;
	       ));
     args = select(args, arg -> class arg =!= Option);
     if not opts.?Key then error "missing Key";
     key := normalizeDocumentTag opts.Key;
     verifyTag key;
     if not isDocumentableTag key then error("undocumentable item encountered");
     opts.FormattedKey = currentNodeName = formatDocumentTag key;
     pkg := packageTag key;
     if pkg =!= currentPackage then error("documentation for \"",key,"\" belongs in package ",pkg," but current package is ",currentPackage);
     if #args > 0 then (
	  if opts.?Description then error "Description option provided, as well as items in the list";
	  opts.Description = toList args;
	  );
     exampleBaseFilename = makeFileName(currentNodeName,if opts.?FileName then opts.FileName,currentPackage);
     if currentPackage === null then error "documentation encountered outside a package";
     if currentPackage#"documentation"#?currentNodeName then duplicateDocWarning();
     opts = new HashTable from opts;
     opts = applyPairs(opts,(key,val) -> (key,fixupTable#key val));
     currentPackage#"documentation"#currentNodeName = opts;
     currentNodeName = null;
     )

-----------------------------------------------------------------------------
-- getting help from the documentation
-----------------------------------------------------------------------------

topicList = () -> sort flatten apply(values PackageDictionary, p -> keys (value p)#"documentation")

getExampleInputs := method(SingleArgumentDispatch => true)
getExampleInputs Thing        := t -> {}
getExampleInputs ExampleTABLE := t -> apply(toList t, first)
getExampleInputs MarkUpList   := t -> join apply(toSequence t, getExampleInputs)

examples = x -> getExampleInputs documentation x
printExamples = f -> scan(examples f, i -> << i << endl)
topics = Command (() -> pager columnate(if printWidth != 0 then printWidth else 80, format \ topicList()))
apropos = (pattern) -> sort select(flatten \\ keys \ globalDictionaries, i -> match(toString pattern,i))
-----------------------------------------------------------------------------
-- more general methods
-----------------------------------------------------------------------------
lookupQ := s -> (youngest s)#?s
nextMoreGeneral2 := (f,A) -> (
     A' := A;
     l := false;
     while not l and A' =!= Thing do (
	  A' = parent A';
	  l = lookupQ(f,A');
	  );
     if l then (f,A'))
nextMoreGeneral3 := (f,A,B) -> (
     A' := A;
     B' := B;
     l := false;
     while not l and (A',B') =!= (Thing,Thing) do (
	  if B' =!= Thing then B' = parent B' else (
	       B' = B;
	       A' = parent A');
	  l = lookupQ(f,A',B'););
     if l then (f,A',B'))
nextMoreGeneral4 := (f,A,B,C) -> (
     A' := A;
     B' := B;
     C' := C;
     l := false;
     while not l and (A',B',C') =!= (Thing,Thing,Thing) do (
	  if C' =!= Thing then C' = parent C'
	  else (
	       C' = C;
	       if B' =!= Thing then B' = parent B'
	       else (
		    B' = B;
		    A' = parent A'));
	  l = lookupQ(f,A',B',C');
	  );
     if l then (f,A',B',C'))
nextMoreGeneral := s -> (
     if class s === Sequence then (
	  if #s === 0 then null 
	  else if class s#-1 === Symbol then unSingleton drop(s,-1) -- dropping optional argument tag
	  else if #s === 2 then nextMoreGeneral2 s 
	  else if #s === 3 then nextMoreGeneral3 s 
	  else if #s === 4 then nextMoreGeneral4 s))

evenMoreGeneral := key -> (
     t := nextMoreGeneral key;
     if t === null and class key === Sequence then key#0 else t)

headline = memoize (
     key -> (
	  while (
	       d := getOption(key,Headline);
	       d === null
	       )
	  and ( 
	       key = evenMoreGeneral key;
	       key =!= null
	       )
	  do null;
	  if d =!= null then d))

commentize := s -> if s =!= null then concatenate(" -- ",s)

moreGeneral := s -> (
     n := nextMoreGeneral s;
     if n =!= null then SEQ { "Next more general method: ", TO n, commentize headline n }
     )

-----------------------------------------------------------------------------

optTO := i -> if getDoc i =!= null then SEQ{ TO i, commentize headline i } else formatDocumentTagTO i
optTOCLASS := i -> if getDoc i =!= null then SEQ{ TO i, " (", OFCLASS class value i, ")", commentize headline i } else formatDocumentTagTO i

smenu := s -> UL (optTO \ last \ sort apply(s , i -> {formatDocumentTag i, i}) )
smenuCLASS := s -> UL (optTOCLASS \ last \ sort apply(s , i -> {formatDocumentTag i, i}) )
 menu := s -> UL (optTO \ s)

vowels := set characters "aeiouAEIOU"
indefiniteArticle := s -> if vowels#?(s#0) and not match("^one ",s) then "an " else "a "
indefinite := s -> concatenate(indefiniteArticle s, s)
synonym = X -> if X.?synonym then X.synonym else "object of class " | toString X

synonymAndClass := X -> (
     if X.?synonym then SEQ {indefinite X.synonym, " (of class ", TO X, ")"}
     else SEQ {"an object of class ", TO X}
     )     

justClass := X -> SEQ {"an instance of class ", TO X}

OFCLASS = X -> (
     if parent X === Nothing then error "expected a class";
     if X.?synonym then SEQ {indefiniteArticle X.synonym, TO2 {X, X.synonym}}
     else SEQ {"an object of class ", TO X}
     )

makeDocBody := method(SingleArgumentDispatch => true)
makeDocBody Thing := key -> (
     fkey := formatDocumentTag key;
     pkg := getPackage fkey;
     if pkg =!= null then (
	  rec := getRecord(pkg,fkey);
	  docBody := extractBody rec;
	  if docBody =!= null and #docBody > 0 then (
	       docBody = processExamples(pkg, fkey, docBody);
	       if class key === String 
	       then PARA {docBody}
	       else SEQ { PARA BOLD "Description", PARA {docBody} })))

title := s -> PARA { STRONG formatDocumentTag s, commentize headline s }

inlineMenu := x -> between(", ", TO \ x)

type := S -> (
     s := value S;
     PARA { "The object ", TO S, " is ", OFCLASS class s,
     	  if parent s =!= Nothing then (
     	       f := (T -> while T =!= Thing list parent T do T = parent T) s;
	       SEQ splice {
		    if #f>1 then ", with ancestor classes " else if #f == 1 then ", with ancestor class " else ", with no ancestor class.", 
		    toSequence between(" < ", f / (T -> TO T)) 
		    }
	       ),
	  "."
     	  }
     )

istype := X -> parent X =!= Nothing
alter1 := x -> (
     if class x === Option and #x === 2 then (
	  if istype x#0 then SEQ { OFCLASS x#0, if x#1 =!= "" and x#1 =!= null then SEQ { ", ", x#1 } }
	  else error "expected type to left of '=>'"
	  )
     else x)
alter := x -> (
     if class x === Option and #x === 2 then (
	  if istype x#0 then SEQ { OFCLASS x#0, if x#1 =!= "" and x#1 =!= null then SEQ { ", ", x#1 } }
	  else if class x#0 === String then (
	       if class x#1 === Option and #x#1 === 2 then (
		    if istype x#1#0 then SEQ { TT x#0, ", ", OFCLASS x#1#0, if x#1#1 =!= "" and x#1#1 =!= null then SEQ { ", ", x#1#1 } }
		    else error "expected type to left of '=>'"
		    )
	       else SEQ { TT x#0, if x#1 =!= "" and x#1 =!= null then SEQ { ", ", x#1 } }
	       )
	  else error "expected string or type to left of '=>'"
	  )
     else SEQ x)

typicalValue := k -> (
     if typicalValues#?k then typicalValues#k 
     else if class k === Sequence and typicalValues#?(k#0) then typicalValues#(k#0)
     else Thing
     )

types := method(SingleArgumentDispatch => true)
types Thing := x -> ({},{})
types Function := x -> ({},{typicalValue x})
types Sequence := x -> (
     if #x > 1 and instance(x#-1,Symbol) 
     then ({},{})					    -- it's an option ...
     -- then types unSingleton drop(x,-1)
     else ( drop(toList x,1), { typicalValue x } ))

isopt := x -> class x === Option and #x === 2

merget := (v,v') -> apply(v,v',(a,t) -> (
	  if t =!= Thing then (
	       if isopt a then (
		    if isopt a#1 then (
			 if a#1#0 =!= t then error "type mismatch"
			 else a
			 )
		    else (
			 if istype a#0 then (
			      if a#0 =!= t then error "type mismatch"
			      else a
			      )
			 else a#0 => t => a#1			      
			 )
		    )
	       else t => a)
	  else a))

optargs := method(SingleArgumentDispatch => true)
optargs Thing := x -> null
optargs Function := f -> (
     o := options f;
     if o =!= null then PARA { "Optional arguments :", smenu apply(keys o, t -> f => t)})
optargs Sequence := s -> (
     o := options s;
     if o =!= null then PARA { "Optional arguments :", smenu apply(keys o, t -> s => t)}
     else optargs s#0)

optin0 := new OptionTable from {}
optin := method(SingleArgumentDispatch => true)
optin Thing := x -> optin0
optin Function := f -> (
     o := options f;
     if o =!= null then o else optin0)
optin Sequence := s -> (
     o := options s;
     if o =!= null then o else optin s#0)

synopsisOpts := new OptionTable from {			    -- old
     Usage => null,
     Function => null,
     Inputs => {},
     Outputs => {},
     Results => {}
     }
synopsis := method(SingleArgumentDispatch => true)
synopsis Thing := key -> (
     -- we still want to put
     --	       moreGeneral s
     -- back somewhere....
     o := getDoc key;
     if o === null then o = synopsisOpts;
     inp := if o.?Inputs then o.Inputs else {};
     out := if o.?Outputs then o.Outputs else {};
     res := if o.?Results then o.Results else {};
     usa := if o.?Usage then o.Usage;
     fun := if o#?Function then o#Function;
     iso := x -> instance(x,Option) and #x==2 and instance(x#0,Symbol);
     if class inp === SEQ then inp = toList inp;
     ino := select(inp, x -> iso x);
     opt := optin key;
     ino = new HashTable from toList ino;
     ino = apply(sort pairs opt, (tag,dft) -> (
	       if ino#?tag 
	       then SEQ { TO toString tag, " => ", alter1 ino#tag, " [", net dft, "]" }
	       else SEQ { TO toString tag, " => ... [", net dft, "]" }
	       ));	       
     inp = select(inp, x -> not iso x);
     (inp',out') := types key;
     if out' === {Thing} then out' = {};		    -- not informative enough
     if #inp === 0 then (
	  inp = apply(inp', T -> T => "");
	  )
     else if #inp' =!= 0 then (
     	  if #inp =!= #inp' then error "mismatched number of inputs";
     	  inp = merget(inp,inp');
	  );
     if class out === SEQ then out = toList out;
     if #out === 0 then (
	  out = apply(out', T -> T => "");
	  )
     else if #out' =!= 0 then (
     	  if #out =!= #out' then error "mismatched number of outputs";
     	  out = merget(out,out');
	  );
     inp = alter \ inp;
     out = alter \ out;
     if #inp > 0 or #ino > 0 or #out > 0 then (
	  SEQ {						    -- to be implemented
     	       PARA BOLD "Synopsis",
	       UL {
     	       	    if usa =!= null then PARA { "Usage: ", if class usa === String then TT usa else usa},
		    if fun =!= null then SEQ { "Function: ", TO fun }
		    else if class key === Sequence and key#?0 then (
	       		 if class key#0 === Function 
			 then SEQ { "Function: ", TO key#0 }
			 else SEQ { "Operator: ", TO key#0 }
			 ),
		    if inp#?0 then PARA { "Inputs:", UL inp },
		    if ino#?0 then PARA { "Optional inputs:", UL ino },
		    if out#?0 then PARA { "Outputs:", UL out },
		    if res#?0 then PARA { "Results:", UL res }
		    }
	       }
	  ))

documentableMethods := s -> select(methods s, isDocumentableTag)

fmeth := f -> (
     b := documentableMethods f;
     if methodFunctionOptions#?f and not methodFunctionOptions#f.SingleArgumentDispatch
     then b = select(b, x -> x =!= (f,Sequence));
     if #b > 0 then SEQ { PARA { "Ways to use ", TT toString f }, smenu b } )

noBriefDocThings := hashTable { symbol <  => true, symbol >  => true, symbol == => true }
briefDocumentation = method(SingleArgumentDispatch => true)

briefDocumentation VisibleList := x -> null

briefDocumentation Thing :=
-- briefDocumentation File := 
-- briefDocumentation BasicList := 
-- briefDocumentation Function := 
-- briefDocumentation MutableHashTable := 
-- briefDocumentation HashTable := 
x -> (
     if noBriefDocThings#?x or not isDocumentableThing x then return null;
     r := synopsis x;
     if r =!= null then << endl << r << endl
     else (
	  if headline x =!= null then << endl << commentize headline x << endl;
	  if class x === Function then (
	       s := fmeth x;
	       if s =!= null then << endl << s << endl;)))

documentation = method(SingleArgumentDispatch => true)
documentation String := key -> (
     if unformatTag#?key then documentation unformatTag#key 
     else if isGlobalSymbol key then (
	  t := getGlobalSymbol key;
	  documentation t)
     else (
	  b := makeDocBody key;
	  if b === null then null
	  else Hypertext join({title key}, b, {caveat key, seealso key, theMenu key})))

binary := set binaryOperators; erase symbol binaryOperators
prefix := set prefixOperators; erase symbol prefixOperators
postfix := set postfixOperators; erase symbol postfixOperators
other := set otherOperators; erase symbol otherOperators
operatorSet = binary + prefix + postfix + other
op := s -> if operatorSet#?s then (
     ss := toString s;
     SEQ {
	  if binary#?s then PARA {
	       "This operator may be used as a binary operator in an expression \n",
	       "like ", TT ("x "|ss|" y"), ".  The user may install ", TO {"binary methods"}, " \n",
	       "for handling such expressions with code such as ",
	       if ss == " "
	       then PRE ("         X Y := (x,y) -> ...")
	       else PRE ("         X "|ss|" Y := (x,y) -> ..."), 
	       "where ", TT "X", " is the class of ", TT "x", " and ", TT "Y", " is the \n",
	       "class of ", TT "y", "."
	       },
	  if prefix#?s then PARA {
	       "This operator may be used as a prefix unary operator in an expression \n",
	       "like ", TT (ss|" y"), ".  The user may install a method for handling \n",
	       "such expressions with code such as \n",
	       PRE ("           "|ss|" Y := (y) -> ..."),
	       "where ", TT "Y", " is the class of ", TT "y", "."
	       },
	  if postfix#?s then PARA {
	       "This operator may be used as a postfix unary operator in an expression \n",
	       "like ", TT ("x "|ss), ".  The user may install a method for handling \n",
	       "such expressions with code such as \n",
	       PRE ("         X "|ss|"   := (x,y) -> ..."),
	       "where ", TT "X", " is the class of ", TT "x", "."
	       },
	  }
     )

optionFor := s -> unique select( value \ flatten(values \ globalDictionaries), f -> class f === Function and (options f)#?s) -- this is slow!

ret := k -> (
     t := typicalValue k;
     if t =!= Thing then PARA {"Class of returned value: ", TO t, commentize headline t}
     )
seecode := x -> (
     f := lookup x;
     n := code f;
     if n =!= null 
     -- and height n + depth n <= 10 
     and width n <= maximumCodeWidth
     then PARA { BOLD "Code", PRE demark(newline,unstack n) }
     )

documentationValue := method()
documentationValue(Symbol,Function) := (s,f) -> SEQ { 
     ret f, 
     fmeth f,
     -- seecode f 
     }
documentationValue(Symbol,Type) := (s,X) -> (
     syms := unique flatten(values \ globalDictionaries);
     a := apply(select(pairs typicalValues, (key,Y) -> Y===X and isDocumentableTag key), (key,Y) -> key);
     b := toString \ select(syms, y -> instance(value y, Type) and parent value y === X);
     c := select(documentableMethods X, key -> not typicalValues#?key or typicalValues#key =!= X);
     e := toString \ select(syms, y -> not mutable y and class value y === X);
     SEQ {
	  if #b > 0 then SEQ { 
	       PARA {"Types of ", if X.?synonym then X.synonym else toString X, " :"},
	       smenu b},
	  if #a > 0 then PARA {"Functions and methods returning ", indefinite synonym X, " :", smenu a },
	  if #c > 0 then PARA {"Methods for using ", indefinite synonym X, " :", smenu c},
	  if #e > 0 then PARA {"Fixed objects of class ", toString X, " :", smenu e},
	  })
documentationValue(Symbol,HashTable) := (s,x) -> (
     c := documentableMethods x;
     SEQ { if #c > 0 then PARA {"Functions installed in ", toString x, " :", smenu c}})
documentationValue(Symbol,Thing) := (s,x) -> SEQ { }
documentationValue(Symbol,Package) := (s,pkg) -> (
     e := pkg#"exported symbols";
     a := select(e,x -> instance(value x,Function));	    -- functions
     b := select(e,x -> instance(value x,Type));	    -- types
     m := unique flatten apply(b, T -> select(keys value T, 
	       i -> class i === Sequence and (
		    class i#0 === Symbol
		    or
		    class i#0 === Function and ReverseDictionary#?(i#0) -- some method functions are local to the package, thus not visible
		    ))); -- methods
     c := select(e,x -> instance(value x,Symbol));	    -- symbols
     d := toList(set e - set a - set b - set c);	    -- other things
     fn := pkg#"title" | ".m2";
     SEQ {
	  PARA BOLD "Version", PARA { "This documentation describes version ", pkg.Options.Version, " of the package." },
	  PARA BOLD "Source code", PARA { "The source code is in the file ", HREF { LAYOUT#"packages" | fn, fn }, "." },
	  if #pkg#"exported symbols" > 0 then PARA {
	       BOLD "Exports",
	       UL {
		    if #b > 0 then SEQ {"Types", smenu b},
		    if #a > 0 then SEQ {"Functions", smenu a},
		    if #m > 0 then SEQ {"Methods", smenu m},
		    if #c > 0 then SEQ {"Symbols", smenu c},
		    if #d > 0 then SEQ {"Other things", smenuCLASS d},
		    }
	       },
	  }
     )

documentation Symbol := S -> (
     a := apply(select(optionFor S,f -> isDocumentableTag f), f -> f => S);
     b := documentableMethods S;
     Hypertext {
	  title S, 
	  synopsis S,
	  makeDocBody S,
	  op S,
	  if #a > 0 then PARA {"Functions with optional argument named ", toExternalString S, " :", smenu a},
	  if #b > 0 then PARA {"Methods for ", toExternalString S, " :", smenu b},
     	  documentationValue(S,value S),
	  type S,
	  caveat S,
	  seealso S,
	  theMenu S
     	  }
     )

documentation Sequence := key -> (
     if key#?-1 and instance(key#-1,Symbol) then (		    -- optional argument
	  fn := unSingleton drop(key,-1);
	  opt := key#-1;
	  if not (options fn)#?opt then error ("function ", fn, " does not accept option key ", opt);
	  default := (options fn)#opt;
	  Hypertext { 
	       title key,
	       synopsis key,
	       makeDocBody key,
	       caveat key,
	       PARA BOLD "Further information", UL {
		    SEQ{ "Default value: ", if hasDocumentation default then TOH default else TT default },
		    SEQ{ if class fn === Sequence then "Method: " else "Function: ", TOH fn },
		    SEQ{ "Option name: ", TOH opt }
		    },
	       seealso key,
	       theMenu key
	       }
	  )
     else (						    -- method key
	  if null === lookup key then error("expected ", toString key, " to be a method");
	  Hypertext {
	       title key, 
	       synopsis key,
	       makeDocBody key,
	       caveat key,
	       seealso key,
	       theMenu key
	       }
	  ))

documentation Thing := x -> if ReverseDictionary#?x then return documentation ReverseDictionary#x else SEQ{ " -- undocumented -- "}

hasDocumentation = x -> (
     fkey := formatDocumentTag x;
     p := select(value \ values PackageDictionary, P -> P#"documentation"#?fkey);
     0 < #p)

pager = x -> (
     if height stdio > 0
     then "!" | (if getenv "PAGER" == "" then "more" else getenv "PAGER") << x << close 
     else << x << endl ;)
help = method(SingleArgumentDispatch => true)
help List := v -> (
     printWidth = printWidth - 2;
     r := boxList apply(v, x -> net documentation x);
     printWidth = printWidth + 2;
     pager r)
help Thing := s -> (
     if s === () then s = "initial help";
     r := documentation s;
     if r === null then r = Hypertext { "No documentation found for '", formatDocumentTag s, "'"};
     pager net r)
help = Command help

-----------------------------------------------------------------------------
-- helper functions useable in documentation
-----------------------------------------------------------------------------

numtests := 0

TEST = method()
TEST Function := TEST String := s -> (
     x := currentPackage#"test inputs";
     x# #x = s;
     )
TEST List := y -> TEST \ y

-----------------------------------------------------------------------------

dummyDoc := x -> document {
     if value x =!= x and (
	  class value x === Function
	  or class value x === ScriptedFunctor
	  or instance(value x, Type)
	  )
     then value x
     else x,
     Headline => "undocumented symbol", "No documentation provided yet."}

undocErr := x -> (
     pos := locate x;
     pos = if pos === null then "error: " else pos#0 | ":" | toString pos#1 | ": ";
     stderr << pos << x << " undocumented " << synonym class value x << endl;
     )

undocumentedSymbols = () -> select(
     flatten(values \ globalDictionaries), 
     x -> (
	  if (
	       -- x =!= value x and        -- ignore symbols with no value assigned
	       not DocDatabase#?(toString x)
	       ) 
     	  then (
	       undocErr x;
	       dummyDoc x;
	       true)))

-----------------------------------------------------------------------------

new TO from Sequence := new TOH from Sequence := (TO,x) -> new TO from {x}
new TO from List := (TO,x) -> (
     verifyTag first x;
     x)
new IMG from List := (IMG,x) -> (
     url := first x;
     --   It's hard to know where the html file will go, and if the path to image is relative
     --   then we can't check it.
     -- if not isAbsolute url and not fileExists url then error ("file ", url, " does not exist");
     x)

-- Local Variables:
-- compile-command: "make -C $M2BUILDDIR/Macaulay2/m2 "
-- End:
