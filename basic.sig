-- generated by scc1

signature basic (
     use system;
     use stdio;
     use stdiop;
     use binding;
     use system;
     use strings;
     use nets;
     use tokens;
     use err;
     use stdio;
     use arithmetic;
     import hash(e:Expr):int;
     import hash(x:List):int;
     import sethash(x:List,mutable:bool):List;
     import setmutability(x:List,mutable:bool):List;
     import hash(x:Object):int;
     import sethash(o:Object,mutable:bool):Object;
     import setmutability(o:Object,mutable:bool):Object;
     import copy(v:Sequence):Sequence;
     import copy(table:array(KeyValuePair)):array(KeyValuePair);
     import copy(obj:Object):Object;
     import copy(a:List):List;
     import reverse(a:Sequence):Sequence;
     import reverse(a:List):List;
     import seq():Expr;
     import seq(e:Expr,f:Expr):Expr;
     import seq(e:Expr,f:Expr,g:Expr):Expr;
     import list(a:Sequence):Expr;
     import list(class:Object,a:Sequence):Expr;
     import list(class:Object,a:Sequence,mutable:bool):Expr;
     import list(class:Object,e:Expr):Expr;
     import emptylist:Expr;
     import list():Expr;
     import list(e:Expr,f:Expr):Expr;
     import list(e:Expr,f:Expr,g:Expr):Expr;
     import list(e:Expr,f:Expr,g:Expr,h:Expr):Expr;
     import Array(a:Sequence):Expr;
     import Array(e:Expr):Expr;
     import emptyArray:Expr;
     import Array():Expr;
     import Array(e:Expr,f:Expr):Expr;
     import Array(e:Expr,f:Expr,g:Expr):Expr;
     import Array(e:Expr,f:Expr,g:Expr,h:Expr):Expr;
);
