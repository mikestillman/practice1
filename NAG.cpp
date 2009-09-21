// Copyright 2008 Anton Leykin and Mike Stillman

#include "NAG.hpp"
#include "matrix-con.hpp"
#include <dlfcn.h>
#include <time.h>
#include "lapack.hpp"

 
// Straiight Line Program class

StraightLineProgram::StraightLineProgram()
{
  handle = NULL;
  eval_time = 0;
  n_calls = 0;
}

StraightLineProgram::~StraightLineProgram()
{ 
  if (handle!=NULL) {
    printf("closing library\n");
    dlclose(handle);
  }
}

int StraightLineProgram::num_slps = 0;
StraightLineProgram* StraightLineProgram::catalog[MAX_NUM_SLPs];

StraightLineProgram_OrNull *StraightLineProgram::make(Matrix *m_consts, M2_arrayint program)
{
  // todo: move some of these lines to rawSLP
  StraightLineProgram* res;
  if (num_slps>MAX_NUM_SLPs) {
    ERROR("max number of slps exceeded");
    res = NULL;
  };
  if (program->len < 3) {
    ERROR("invalid SLP");
    res = NULL;
  } else {
    res = new StraightLineProgram;
    catalog[num_slps++] = res;
    res->is_relative_position = false;
    res->num_consts = program->array[0];
    if (m_consts->n_cols() != res->num_consts) 
      ERROR("different number of constants expected");
    res->num_inputs = program->array[1];
    res->rows_out = program->array[2];
    res->cols_out = program->array[3];
    res->program = program;
    switch (program->array[4]) {
    case slpCOMPILED: {
      // nodes = constants + input + output
      res->nodes = newarray_atomic(complex, res->num_consts + res->num_inputs + res->rows_out*res->cols_out);
      char libname[100]; 
      sprintf(libname, "%s%d.dylib", libPREFIX, program->array[5]);//Mac OS
      //sprintf(libname, "%s%d.so", libPREFIX, program->array[5]);//Linux (not working yet)
      const char *funname = "slpFN";
      printf("loading slpFN from %s\n", libname);
      res->handle = dlopen(libname, RTLD_LAZY | RTLD_GLOBAL);
      if (res->handle == NULL) ERROR("can't load library %s", libname);
      res->compiled_fn = (void (*)(complex*,complex*)) dlsym(res->handle, funname);
      if (res->compiled_fn == NULL) ERROR("can't link function %s from library %s",funname,libname);
    } break;
    case slpPREDICTOR: 
    case slpCORRECTOR:
      res->nodes = newarray_atomic(complex, res->num_consts+res->num_inputs+res->rows_out*res->cols_out);
      break;
    default: res->nodes = newarray_atomic(complex, program->len+res->num_consts+res->num_inputs);
    }
    for (int i=0; i<res->num_consts; i++) { 
      res->nodes[i] = complex(BIGCC_VAL(m_consts->elem(0,i)));
    }
  }
  return res;
}

StraightLineProgram_OrNull *StraightLineProgram::copy()
{
  StraightLineProgram* res = new StraightLineProgram;
  res->is_relative_position = is_relative_position;
  res->program = makearrayint(program->len);
  memcpy(res->program->array, program->array, program->len*sizeof(int));
  res->nodes = newarray_atomic(complex, num_consts+num_inputs+num_operations);
  for(int i=0; i<num_consts; i++)
      res->nodes[i] = nodes[i];
  res->node_index = node_index; // points to position in program (rel. to start) of operation correspoding to a node
  res->num_consts = num_consts;
  res->num_inputs = num_inputs;
  res->num_operations = num_operations;
  res->rows_out = rows_out;
  res->cols_out = cols_out;
  res->handle = handle;
  res->compiled_fn = compiled_fn;
  res->eval_time = eval_time;
  res->n_calls = n_calls;
  return res;
}

Nterm * extract_divisible_by_x(Nterm *&ff, int i) // auxiliary
{
  /* Extracts into fx the terms divisible by the (n-1-i)-th variable "x"
     and divides them by x. (exponent vectors are assumed to be reversed)
     Note: terms in fx may not be in monomial order. */
  Nterm *fx = NULL;
  Nterm *f = ff;
  Nterm *prev_f = NULL; 
  while (f != NULL) {
    if (f->monom[i] == 0) {
	  prev_f = f;
	  f = f->next;
    } else {
      f->monom[i]--; // divide by x
      if (prev_f != NULL) {
	prev_f->next = f->next; // extract
      } else {
	ff = f->next; // ... or cut
      }
      f->next = fx; // prepend to fx
      fx = f;
      f = (prev_f==NULL)?ff:prev_f->next; 
    }
  }
  return fx;
}

int add_constant_get_position(array<complex>& consts, complex c) //auxiliary
{
  //!!! smarter implementation coming !!!
  consts.append(c);
  return consts.length()-1;
}

/* create the part of slp computing f, return the position of the final operation */
int StraightLineProgram::poly_to_horner_slp(int n, intarray& prog, array<complex>& consts, Nterm *&f) // auxiliary
{
  int part_pos[n]; // absolute positions of the parts
  int last_nonzero_part_pos = ZERO_CONST;
  for (int i=0; i<n; i++) {
    Nterm* fx = extract_divisible_by_x(f,i);
    if (fx==NULL) 
      part_pos[i] = ZERO_CONST;
    else {  
      int p = poly_to_horner_slp(n,prog,consts,fx);
      last_nonzero_part_pos = part_pos[i] = num_operations++;
      node_index.append(prog.length());
      prog.append(slpPRODUCT);
      prog.append(n-1-i); // reference to (n-1-i)-th input (recall: the order of vars is reversed in monomials)
      prog.append(p - part_pos[i]); // relative position of p  
    }
  }
  int c = 0; // count nonzeros
  if (f!=NULL) 
    c++;
  for (int i=0; i<n; i++) 
    if (part_pos[i] != ZERO_CONST) 
      c++;
  if (c==0) 
    return ZERO_CONST;
  if (c==1 && last_nonzero_part_pos != ZERO_CONST) 
    return last_nonzero_part_pos;
  int cur_p = num_operations++; 
  node_index.append(prog.length());
  prog.append(slpMULTIsum);
  prog.append(c); 
  if (f!=NULL) 
    prog.append(CONST_OFFSET + add_constant_get_position(consts,complex(BIGCC_VAL(f->coeff))));
  for (int i=0; i<n; i++) 
    if (part_pos[i] != ZERO_CONST) 
      prog.append(part_pos[i] - cur_p); // relative position of the i-th part
  return cur_p;
}

void monomials_to_conventional_exponent_vectors(int n, Nterm *f) //auxiliary
/* "unpack" monomials */ 
{
  for (; f!=NULL; f=f->next)
    for (int i=0; i<n-1; i++)
      f->monom[i] -= f->monom[i+1];
}


StraightLineProgram_OrNull *StraightLineProgram::make(const PolyRing *R, ring_elem e)
{
  // todo: move some of these lines to rawSLP
  StraightLineProgram* res;
  if (num_slps>MAX_NUM_SLPs) {
    ERROR("max number of slps exceeded");
    res = NULL;
  } else {
    res = new StraightLineProgram;
    res->is_relative_position = true;
    int n = res->num_inputs = R->n_vars();
    res->num_operations = 0;
    res->rows_out = 1;
    res->cols_out = 1;
    intarray prog;
    array<complex> consts;
 
    // make prog and node
    e = R->copy(e); /* a copy of "e" will be decomposed; 
		       how to remove the pieces afterwards? 
		       R->remove(...) is an empty function */
    Nterm* f = e.get_poly();
    monomials_to_conventional_exponent_vectors(n,f);
    int out = res->poly_to_horner_slp(n, prog, consts, f);
    if (out==ZERO_CONST) {
      out = res->num_operations++; 
      res->node_index.append(prog.length());
      prog.append(slpMULTIsum);
      prog.append(0); // sum with zero summands
    }

    // make program
    res->program = makearrayint(prog.length() + 2/* accounts for lines +2,+3 */ + SLP_HEADER_LEN);
    res->program->array[0] = res->num_consts = consts.length();
    prog.append(slpEND);
    prog.append(out+res->num_consts+res->num_inputs); // position of the output

    res->program->array[1] = res->num_inputs;
    res->program->array[2] = res->rows_out;
    res->program->array[3] = res->cols_out;
    memcpy(res->program->array+SLP_HEADER_LEN, prog.raw(), sizeof(int)*prog.length());
    
    // make nodes: [constants, inputs, operations]
    res->nodes = newarray_atomic(complex, res->num_consts+res->num_inputs+res->num_operations);
    for(int i=0; i<res->num_consts; i++)
      res->nodes[i] = consts[i];
  }
  return res;
}

StraightLineProgram_OrNull *StraightLineProgram::concatenate(const StraightLineProgram* slp)
{
  if (!is_relative_position || !slp->is_relative_position  
      || num_inputs!=slp->num_inputs || rows_out!=slp->rows_out) 
    {
      ERROR("slps unstackable");
      return NULL;
    }
  int num_outputs = rows_out*cols_out;
  int *end_program = program->array+program->len - num_outputs;
  int *slp_start_program = slp->program->array+SLP_HEADER_LEN;
  int slp_num_outputs = slp->rows_out*slp->cols_out;

  int slp_len = slp->program->len - SLP_HEADER_LEN - slp_num_outputs; 
  // length of the part containing operations and slpEND
  
  StraightLineProgram* res = new StraightLineProgram;
  res->is_relative_position = true;
  res->num_inputs = num_inputs;
  res->num_operations = num_operations + slp->num_operations;
  res->rows_out = rows_out;
  res->cols_out = cols_out + slp->cols_out;
  
  //  array<complex> consts; // !!! use to optimize constants
  
  // make program
  res->program = makearrayint(program->len + slp_len - 1 + slp_num_outputs);
  res->program->array[1] = res->num_inputs;
  res->program->array[2] = res->rows_out;
  res->program->array[3] = res->cols_out;
  
  int* res_start_program = res->program->array + SLP_HEADER_LEN; 
  int* res_end_program = res->program->array + res->program->len - slp_num_outputs - num_outputs; 

  // copy "this"
  memcpy(res_start_program, program->array + SLP_HEADER_LEN, program->len*sizeof(int));
  memcpy(res_end_program, end_program, num_outputs*sizeof(int));
  for (int *a = res_end_program, i=0; i<num_outputs; i++, a++)  
    *a += slp->num_consts;  //!!! assume: appending constants
  res->node_index = node_index;

  // copy "slp"
  memcpy(res_end_program-slp_len, slp_start_program, slp_len*sizeof(int));    
  for (int *a = res_end_program-slp_len, i=0; i<slp_len - 1/* for slpEND */; i++, a++)  
    if (*a >= CONST_OFFSET) // if refers to a constant
      *a += num_consts; //!!! assume: appending constants
  memcpy(res_end_program+num_outputs, slp_start_program+slp_len, slp_num_outputs*sizeof(int));
  for (int *a = res_end_program+num_outputs, i=0; i<slp_num_outputs; i++, a++)  
    *a += num_consts+num_operations;  //!!! assume: appending constants
  for (int i=0; i<slp->num_operations; i++)
    // shift by the size of "operations" part of this->program
    res->node_index.append(slp->node_index[i] +  program->len - SLP_HEADER_LEN - num_outputs - 1);

  res->program->array[0] = res->num_consts = num_consts + slp->num_consts; //!!! assume: appending constants
  
  // make nodes: [constants, inputs, operations]
  res->nodes = newarray_atomic(complex, res->num_consts+res->num_inputs+res->num_operations);
  memcpy(res->nodes, nodes, num_consts*sizeof(complex)); //!!! assume: appending constants
  memcpy(res->nodes+num_consts, slp->nodes, slp->num_consts*sizeof(complex)); //!!! assume: appending constants
  return res;
}

/* ref = reference to a node rel. n */
int StraightLineProgram::diffPartReference(int n, int ref, int v, intarray& prog)
{
  if (ref<0) 
    return diffNodeInput(n+ref, v, prog);
  else if (ref<CONST_OFFSET) { // an input                                                                                                                              
    if (ref == v) return ONE_CONST; 
    else return ZERO_CONST;
  }
  else return ZERO_CONST; // a constant
}

int StraightLineProgram::diffNodeInput(int n, int v, intarray& prog) // used by jacobian
{
  int i = node_index[n];
  switch (prog[i]) {
    /* case slpCOPY:
       break;*/
  case slpMULTIsum: 
    {	
      int n_summands = prog[(++i)++];
      int part_pos[n_summands];
      int c = 0; // count nonzeroes
      int last_non_zero = ZERO_CONST;
      for(int j=0; j<n_summands; j++) {
	part_pos[j] = diffPartReference(n,prog[i++], v, prog);
	if (part_pos[j] != ZERO_CONST) {
	  c++;
	  last_non_zero = part_pos[j];
	}
      }
      if (c == 0) 
	return ZERO_CONST;
      if (c == 1) 
	return last_non_zero;
      int cur_p = num_operations++;
      node_index.append(prog.length());
      prog.append(slpMULTIsum);
      prog.append(c);
      for(int j=0; j<n_summands; j++) {
	if (part_pos[j] != ZERO_CONST)
	  prog.append(part_pos[j] - cur_p); // relative position of the j-th part   
      }
      return cur_p;
    }
  case slpPRODUCT: 
    {
      int a = prog[(++i)++];
      int b = prog[i++];
      int da = diffPartReference(n, a, v, prog);
      int db = diffPartReference(n, b, v, prog);
      if (db == ZERO_CONST) {
	if (da == ZERO_CONST) return ZERO_CONST;
	if (da == ONE_CONST) {
	  if (b < 0) // if refers to operation node
	    return n+b; 
	  else if (b >= CONST_OFFSET) { // ... constant
	    int cur_p = num_operations++;
	    node_index.append(prog.length());
	    prog.append(slpCOPY); // is there better way?
	    prog.append(b);
	    return cur_p;
	  } else { // ... input
	    ERROR("input node not expected");
	    return ZERO_CONST;
	  };
      	} else {
	  int cur_p = num_operations++;
	  node_index.append(prog.length());
	  prog.append(slpPRODUCT);
	  prog.append(da - cur_p);
	  if (b < 0) // if refers to an operation node
	    b = n + b - cur_p; // recalculate wrt cur_p
	  prog.append(b);
	  return cur_p;
	}
      } else if (da == ZERO_CONST) { // ... and db != 0
	if (db == ONE_CONST) {
	  if (a < 0) // if refers to an operation node
	    return n+a; 
	  else if (a >= CONST_OFFSET) { // ... constant
	    int cur_p = num_operations++;
	    node_index.append(prog.length());
	    prog.append(slpCOPY); // is there a better way ?
	    if (a < 0) // if refers to an operation node
	      a = n + a - cur_p; // recalculate wrt cur_p
	    prog.append(a);
	    return cur_p;
	  } else { // ... input
	    ERROR("input node not expected");
	    return ZERO_CONST;
	  };
	} else { // db!=0 and db!=1
	  int cur_p = num_operations++;
	  node_index.append(prog.length());
	  prog.append(slpPRODUCT);
	  prog.append(db - cur_p);
	    if (a < 0) // if refers to an operation node
	      a = n + a - cur_p; // recalculate wrt cur_p
	  prog.append(a);
	  return cur_p;
	}
      } else { // da |=0 and db !=0
	int part1 = ZERO_CONST;
	int is_part1_created = (da != ONE_CONST);
	if (is_part1_created) {
	  int cur_p = num_operations++;
	  node_index.append(prog.length());
	  prog.append(slpPRODUCT);
	  prog.append(da - cur_p);
	  if (b < 0) // if refers to an operation node
	    b = n + b - cur_p; // recalculate wrt cur_p
	  prog.append(b);
	  part1 = cur_p;
	}
	int part2 = ZERO_CONST;
	int is_part2_created = (db != ONE_CONST);
	if (is_part2_created) {
	  int cur_p = num_operations++;
	  node_index.append(prog.length());
	  prog.append(slpPRODUCT);
	  prog.append(db - cur_p);
	  if (a < 0) // if refers to an operation node
	    a = n + a - cur_p; // recalculate wrt cur_p
	  prog.append(a);
	  part2 = cur_p;
	}
	int cur_p = num_operations++;
	node_index.append(prog.length());
	prog.append(slpMULTIsum);
	prog.append(2);
	if (is_part1_created) 
	  prog.append(part1 - cur_p);
	else prog.append((b<0)? b + n - cur_p : b);
	if (is_part2_created) 
	  prog.append(part2 - cur_p);
	else prog.append((a<0)? a + n - cur_p : a);
	return cur_p;
      }
    }
  default:
    ERROR("unknown SLP operation");
    printf("i = %d, a[i] = %d\n", i, prog[i]);
    return 0;
  }
}

StraightLineProgram_OrNull *StraightLineProgram::jacobian(bool makeHxH, StraightLineProgram *&slpHxH, bool makeHxtH, StraightLineProgram *&slpHxtH)
  /* Produces a jacobian of the slp H: (i,j)-th output = dH_j/dx_i
     Constructs also HxH and HxtH. */ 
{
 
  if (rows_out!=1) { 
    ERROR("1-row slp expected");
    return NULL;
  };

  int num_outputs = rows_out*cols_out;
  int *end_program = program->array+program->len - num_outputs;
  
  StraightLineProgram* res = new StraightLineProgram;
  res->is_relative_position = true;
  res->num_inputs = num_inputs;
  res->num_operations = num_operations;
  res->rows_out = num_inputs;
  res->cols_out = cols_out;
  
  //  array<complex> consts; // !!! use to optimize constants
  intarray prog(program->len);
  for (int i=SLP_HEADER_LEN; i < program->len-num_outputs - 1/*for slpEND*/; i++)
    prog.append(program->array[i]);
  res->node_index = node_index;
 
 int out_pos[res->rows_out*res->cols_out]; // records absolute position of output entries
  
  for (int j=0; j<num_outputs; j++)
    for (int i=0; i<num_inputs; i++) 
      out_pos[i*res->cols_out+j] = res->diffNodeInput(end_program[j]-num_consts-num_inputs/*position of j-th output in prog*/,
						      i,prog); //uses res->num_operations
  // make program
  res->program = makearrayint(SLP_HEADER_LEN + prog.length() + 1 + res->rows_out*res->cols_out);
  res->program->array[0] = res->num_consts = num_consts + 1; //!!! assume: appending ZERO
  res->program->array[1] = res->num_inputs;
  res->program->array[2] = res->rows_out;
  res->program->array[3] = res->cols_out;
  prog.append(slpEND);
  for (int i=0; i<num_inputs; i++) 
    for (int j=0; j<num_outputs; j++) {
      int t = out_pos[i*res->cols_out+j];
      prog.append((t==ZERO_CONST)? num_consts/*ref to ZERO*/ : t+res->num_consts+num_inputs); // position of the output
    }
  memcpy(res->program->array+SLP_HEADER_LEN, prog.raw(), sizeof(int)*prog.length());

  // make nodes: [constants, inputs, operations]
  res->nodes = newarray_atomic(complex, res->num_consts+res->num_inputs+res->num_operations);
  memcpy(res->nodes, nodes, num_consts*sizeof(complex)); // old constants
  res->nodes[num_consts] = complex(0,0); // ... plus ZERO

  if (makeHxH) {
    slpHxH = res->copy();
    int *c = slpHxH->program->array + slpHxH->program->len - slpHxH->cols_out;
    for (int j=0; j<num_outputs; j++,c++)
      *c = end_program[j]-num_consts+res->num_consts;
  }
  if (makeHxtH) {
    slpHxtH = res->copy();
    slpHxtH->rows_out++;
    // how to kill M2_arrayint ???
    slpHxtH->program = makearrayint(slpHxtH->program->len + cols_out);
    memcpy(slpHxtH->program->array, res->program->array, sizeof(int)*res->program->len);
    int *c = slpHxtH->program->array + res->program->len;
    for (int j=0; j<num_outputs; j++,c++)
      *c = end_program[j]-num_consts+res->num_consts;
  }
  return res;
}

void StraightLineProgram::evaluate(int n, const complex* values, complex* ret)
{
  if (n != num_inputs) ERROR("wrong number of inputs");
 
  complex* out = NULL; // used by compiledSLP                                                                                                                
  int out_entries_shift = 0; // position of "out matrix"

  int cur_node = num_consts;
  int i;
  copy_complex_array(num_inputs, values, nodes+cur_node);
  cur_node += num_inputs;

  clock_t start_t = clock(); // clock execution time

  switch (program->array[4]) {
  case slpPREDICTOR:
    out = nodes+num_consts+num_inputs;
    predictor();
    break;
  case slpCORRECTOR:
    out = nodes+num_consts+num_inputs;
    corrector();
    break;
  case slpCOMPILED:
    // evaluation via dynamically linked function
    // input: nodes (shifted by number of consts)
    // output: out
    out = nodes+num_consts+num_inputs;
    compiled_fn(nodes+num_consts,out);
    break;
  default: // evaluation by interpretation 
    convert_to_absolute_position();
    i = SLP_HEADER_LEN;
    for(; program->array[i] != slpEND; cur_node++) {
      switch (program->array[i]) {
      case slpCOPY: 
	nodes[cur_node] = nodes[program->array[(++i)++]];
	break;
      case slpMULTIsum: 
	{	
	  int n_summands = program->array[i+1];
	  nodes[cur_node] = (n_summands>0) ?  // create a temp var?  
	    nodes[program->array[i+2]] : complex(0,0); // zero if empty sum
	  for(int j=1; j<n_summands; j++)
	    nodes[cur_node] = nodes[cur_node]+nodes[program->array[i+j+2]];
	  i += n_summands+2; 
	}
	break;
      case slpPRODUCT:
	nodes[cur_node] = nodes[program->array[i+1]] * nodes[program->array[i+2]];
	i+=3;
	break;
      default:
	ERROR("unknown SLP operation");
	return;
      }
    }
    out_entries_shift = i+1;
    //end: evaluation by interpretation
  }

  eval_time += clock()-start_t;
  n_calls++;  

  switch(program->array[4]) {
  case slpPREDICTOR:
  case slpCOMPILED:
    //dynamically linked
    copy_complex_array(rows_out*cols_out, out, ret);
    break;
  default: 
    //interptretation
    complex* c = ret;
    for(i=0; i<rows_out; i++)
      for(int j=0; j<cols_out; j++,c++)  
	*c = nodes[program->array[i*cols_out+j+out_entries_shift]];
    //end: interpretation
  }
}

Matrix *StraightLineProgram::evaluate(const Matrix *values)
{
 
  complex* out = NULL; // used by compiledSLP 
  int out_entries_shift = 0; // position of "out matrix" in slp->program

  int cur_node = num_consts;
  int i;
  if (values->n_cols() != num_inputs) 
    ERROR("different number of inputs expected");
  for(i=0; i<num_inputs; i++, cur_node++)
    nodes[cur_node] = complex(BIGCC_VAL(values->elem(0,i)));

  clock_t start_t = clock(); // clock execution time

  switch (program->array[4]) {
  case slpPREDICTOR:
    out = nodes+num_consts+num_inputs;
    predictor();
    break;
  case slpCORRECTOR:
    out = nodes+num_consts+num_inputs;
    corrector();
    break;
  case slpCOMPILED:
    // evaluation via dynamically linked function
    // input: nodes (shifted by number of consts)
    // output: out
    out = nodes+num_consts+num_inputs;
    compiled_fn(nodes+num_consts,out);
    break;
  default: // evaluation by interpretation 
    convert_to_absolute_position();
    i = SLP_HEADER_LEN;
    for(; program->array[i] != slpEND; cur_node++) {
      switch (program->array[i]) {
      case slpCOPY: 
	nodes[cur_node] = nodes[program->array[(++i)++]];
	break;
      case slpMULTIsum: 
	{	
	  int n_summands = program->array[i+1];
	  nodes[cur_node] = (n_summands>0) ?  // create a temp var?  
	    nodes[program->array[i+2]] : complex(0,0); // zero if empty sum
	  for(int j=1; j<n_summands; j++)
	    nodes[cur_node] = nodes[cur_node]+nodes[program->array[i+j+2]];
	  i += n_summands+2; 
	}
	break;
      case slpPRODUCT:
	nodes[cur_node] = nodes[program->array[i+1]] * nodes[program->array[i+2]];
	i+=3;
	break;
      default:
	ERROR("unknown SLP operation");
	return NULL;
      }
    }
    out_entries_shift = i+1;
    //end: evaluation by interpretation
  }

  eval_time += clock()-start_t;
  n_calls++;

  const CCC* R = values->get_ring()->cast_to_CCC(); 
  //CCC* R = CCC::create(53); //values->get_ring();
  FreeModule* S = R->make_FreeModule(cols_out); 
  FreeModule* T = R->make_FreeModule(rows_out);
  MatrixConstructor mat(T,S);
  mpfr_t re, im;
  mpfr_init(re); mpfr_init(im);
  switch(program->array[4]) {
  case slpPREDICTOR:
  case slpCORRECTOR:
  case slpCOMPILED: {
    //printf("predictor output: %d by %d\n", rows_out, cols_out);
    complex* c = out; 
    for(i=0; i<rows_out; i++)
      for(int j=0; j<cols_out; j++,c++) {
	//printf("%lf %lf \n", c->getreal(), c->getimaginary());
	mpfr_set_d(re, c->getreal(), GMP_RNDN);
	mpfr_set_d(im, c->getimaginary(), GMP_RNDN);
	ring_elem e = R->from_BigReals(re,im);
	mat.set_entry(i,j,e);
      } 
  } break;
  default: //interptretation
    for(i=0; i<rows_out; i++)
      for(int j=0; j<cols_out; j++) {
	complex c = nodes[program->array[i*cols_out+j+out_entries_shift]]; 
	mpfr_set_d(re, c.getreal(), GMP_RNDN);
	mpfr_set_d(im, c.getimaginary(), GMP_RNDN);
	ring_elem e = R->from_BigReals(re,im);
	mat.set_entry(i,j,e);
      }
    //end: interpretation
  }
  mpfr_clear(re); mpfr_clear(im);
  return mat.to_matrix(); 
}


void StraightLineProgram::convert_to_absolute_position()
{
  if (is_relative_position) 
    is_relative_position = false;
  else return;

  int cur_node = num_consts + num_inputs;
  int *a = program->array;
  for(int i = SLP_HEADER_LEN; a[i] != slpEND; cur_node++) {
    switch (a[i]) {
    case slpCOPY:
      relative_to_absolute(a[(++i)++], cur_node); 
      break;
    case slpMULTIsum: 
      {	
	int n_summands = a[(++i)++];
	for(int j=0; j<n_summands; j++)
	  relative_to_absolute(a[i++], cur_node);
      }
      break;
    case slpPRODUCT:
      relative_to_absolute(a[(++i)++], cur_node);
      relative_to_absolute(a[i++],cur_node);
      break;
    default:
      ERROR("unknown SLP operation");
      printf("i = %d, a[i] = %d\n", i, a[i]);
      return;
    }
  }
}

void StraightLineProgram::stats_out(buffer& o) const
{
  o << "Called " << n_calls << " times, total evaluation time = " 
    << (eval_time / CLOCKS_PER_SEC) << "." << (eval_time%CLOCKS_PER_SEC) << " sec" << newline;
}

void StraightLineProgram::text_out(buffer& o) const
{
  if (!is_relative_position) {
    if (program->array[4]==slpCOMPILED) {
      o << "(SLP is precompiled) " << newline;
    }
    if (program->array[4]==slpPREDICTOR || program->array[4]==slpCORRECTOR) {
      return;
    }
  }

  o<<"CONSTANT (count = "<< num_consts;
  o<<") nodes:\n";
  int cur_node = 0;
  int i,j;
  for(i=0; i<num_consts; i++, cur_node++) {
    char s[100];
    nodes[cur_node].sprint(s);
    o << s << ", ";
  }
  o<<newline;   
  o<<"INPUT (count = " << num_inputs <<") nodes:\n";
  for(i=0; i<num_inputs; i++, cur_node++)
    o << cur_node << " ";
  o<<newline;   


  switch (program->array[SLP_HEADER_LEN]) {
  case slpPREDICTOR:
    o << "Predictor: type "  << program->array[5] << newline;
    o << "SLPs: " << program->array[6] << "," << program->array[7] << "," << program->array[8] << newline;
    break;
  default:
    for(i = SLP_HEADER_LEN; program->array[i] != slpEND; cur_node++) {
      o<<cur_node<<" => ";
      switch (program->array[i]) {
      case slpCOPY: 
	o<<"copy "<< program->array[(++i)++];
	break;
      case slpMULTIsum:
	{
	  o<<"sum";
	  int n_summands = program->array[i+1];
	  for(j=0; j<n_summands; j++)
	    o<<" "<<program->array[i+j+2];
	  i += n_summands+2; 
	}
	break;
      case slpPRODUCT:
	o<<"product "<<program->array[i+1]<<" "<<program->array[i+2];
	i+=3;
	break;
      default:
	o<<"BLA i="<<i++;
      }
      o<<newline;
    }
    int out_shift = i+1;
    o<<"OUTPUT ("<< rows_out << "x" << cols_out << ") nodes:\n";
    for(i=0; i<rows_out; i++){
      for(j=0; j<cols_out; j++)
	o << program->array[out_shift+i*cols_out+j] << " ";
      o<<newline;
    }   
  }
}
// end StraightLineProgram


void copy_complex_array(int n, const complex* a, complex* b)
{
  for (int i=0; i<n; i++,a++,b++)
    *b = *a;
}

complex* make_copy_complex_array(int n, const complex* a)
{
  complex* b = newarray_atomic(complex, n);
  for (int i=0; i<n; i++,a++)
    b[i] = *a;
  return b;
}


void multiply_complex_array_scalar(int n, complex* a, const complex b)
{
  for (int i=0; i<n; i++,a++)
    *a = *a * b; 
} 

void add_to_complex_array(int n, complex* a, const complex* b)
{
  for (int i=0; i<n; i++,a++)
    *a = *a + b[i]; 
} 

void negate_complex_array(int n, complex* a)
{
  for (int i=0; i<n; i++,a++)
    *a = -*a; 
} 

double norm2_complex_array(int n, complex* a) // square of 2-norm
{
  double t = 0;
  for (int i=0; i<n; i++,a++)
    t += a->getreal()*a->getreal()+a->getimaginary()*a->getimaginary();
  return t;
}

// BEGIN lapack-based routines

// lapack solve routine (direct call) 
// matrix A is transposed
bool solve_via_lapack(
		   int size, complex* A, // size-by-size matrix of complex #s
		   int bsize, complex* b, // bsize-by-size RHS of Ax=b 
		   complex* x //solution
		   )
{

#if !LAPACK
  ERROR("lapack not present");
  return false;
#else

  bool ret = true;
  int info;

  int *permutation = newarray_atomic(int, size);
  complex* At = newarray(complex, size*size);
  int i,j;
  for(i=0; i<size; i++) for(j=0; j<size; j++) // transpose the matrix: lapack solves A^t x = b
    *(At+i*size+j) = *(A+j*size+i);
  double *copyA = (double*) At; 
  copy_complex_array(size,b,x);
  double *copyb = (double*) x; // result is stored in copyb

  /*
  printf("-----------(solve)-----------------------------------\ncopyA:\n");
  for (i=0; i<size*size; i++)
    printf("(%lf %lf) ", *(copyA+2*i), *(copyA+2*i+1));
  printf("\nb:\n");
  for (i=0; i<size; i++)
    printf("(%lf %lf) ", *(copyb+2*i), *(copyb+2*i+1));
  */
  zgesv_(&size, &bsize,
	 copyA,
	 &size, permutation, 
	 copyb,
	 &size, &info);
  /*
  printf("\nx = b:\n");
  for (i=0; i<size; i++)
    printf("(%lf %lf) ", *(copyb+2*i), *(copyb+2*i+1));
  printf("\n");
  */
  if (info > 0)       
    {
      ERROR("according to zgesv, matrix is singular");
      ret = false;
    }
  else if (info < 0)
    {
      ERROR("argument passed to zgesv had an illegal value");
      ret = false;
    }

  deletearray(permutation);
  deletearray(At);

  return ret;
#endif
}

// lapack solve routine (direct call) 
bool solve_via_lapack_without_transposition(
		   int size, complex* A, // size-by-size matrix of complex #s
		   int bsize, complex* b, // bsize-by-size RHS of Ax=b 
		   complex* x //solution
		   )
{

#if !LAPACK
  ERROR("lapack not present");
  return false;
#else

  bool ret = true;
  int info;

  int *permutation = newarray_atomic(int, size);
  // int i,j;
  double *copyA = (double*) A; 
  copy_complex_array(size,b,x);
  double *copyb = (double*) x; // result is stored in copyb

  /*
  printf("-----------(solve)-----------------------------------\ncopyA:\n");
  for (i=0; i<size*size; i++)
    printf("(%lf %lf) ", *(copyA+2*i), *(copyA+2*i+1));
  printf("\nb:\n");
  for (i=0; i<size; i++)
    printf("(%lf %lf) ", *(copyb+2*i), *(copyb+2*i+1));
  */
  zgesv_(&size, &bsize,
	 copyA,
	 &size, permutation, 
	 copyb,
	 &size, &info);
  /*
  printf("\nx = b:\n");
  for (i=0; i<size; i++)
    printf("(%lf %lf) ", *(copyb+2*i), *(copyb+2*i+1));
  printf("\n");
  */
  if (info > 0)       
    {
      ERROR("according to zgesv, matrix is singular");
      ret = false;
    }
  else if (info < 0)
    {
      ERROR("argument passed to zgesv had an illegal value");
      ret = false;
    }

  deletearray(permutation);

  return ret;
#endif
}

// In: A, a square matrix of size "size"
// Out: true if success, cond = condition number of A
bool cond_number_via_svd(int size, complex* A, double& cond)
{
#if !LAPACK
  ERROR("lapack not present");
  return false;
#else
  bool ret = true;
  char doit = 'A';  // other options are 'S' and 'O' for singular vectors only
  int rows = size;
  int cols = size;
  int info;
  int min = (rows <= cols) ? rows : cols;

  if (min == 0)
    {
      ERROR("expected a matrix with positive dimensions");
      return false;
    }

  int max = (rows >= cols) ? rows : cols;
  int wsize = 4*min+2*max;
  double *workspace = newarray_atomic(double,2*wsize);
  double *rwork = newarray_atomic(double,5*max);

  double *copyA = (double*) A;
  double *u = newarray_atomic(double,2*rows*rows);
  double *vt = newarray_atomic(double,2*cols*cols);
  double *sigma = newarray_atomic(double,2*min);
  
  zgesvd_(&doit, &doit, &rows, &cols, 
	  copyA, &rows,
	  sigma, 
	  u, &rows,
	  vt, &cols,
	  workspace, &wsize, 
	  rwork, &info);

  if (info < 0)
    {
      ERROR("argument passed to zgesvd had an illegal value");
      ret = false;
    }
  else if (info > 0) 
    {
      ERROR("zgesvd did not converge");
      ret = false;
    }
  else
    {
      cond = sigma[size-1]/sigma[0];  
    }

  deletearray(workspace);
  deletearray(rwork);
  //deletearray(copyA);
  deletearray(u);
  deletearray(vt);
  deletearray(sigma);

  return ret;
#endif
}


// END lapack-based routines

void StraightLineProgram::predictor()
{
  int n = num_inputs - 2; // n = size of vectors and matrices 
  const complex* x0t0 = nodes+num_consts;
  const complex* dt = x0t0+n+1;
  complex* dx = nodes+num_consts+num_inputs;
  int predictor_type = program->array[5];
  StraightLineProgram* Hx = catalog[program->array[6]];
  StraightLineProgram* Ht = catalog[program->array[7]];
  //StraightLineProgram* H = catalog[program->array[8]];

  complex* RHS = newarray_atomic(complex, n);
  complex* LHS = newarray_atomic(complex, n*n);
  switch(predictor_type) {
  case TANGENT: {
    Ht->evaluate(n+1,x0t0, RHS);
    multiply_complex_array_scalar(n,RHS,-*dt);
    Hx->evaluate(n+1,x0t0, LHS);
    solve_via_lapack(n,LHS,1,RHS,dx);
  } break;
  case RUNGE_KUTTA: {
    complex one_half(0.5,0);

    complex* xt = newarray_atomic(complex,n+1);
    copy_complex_array(n+1,x0t0,xt);
    complex* dx1 = newarray_atomic(complex,n);
    Ht->evaluate(n+1, xt, RHS);
    negate_complex_array(n,RHS);
    Hx->evaluate(n+1, xt, LHS);
    solve_via_lapack(n,LHS,1,RHS,dx1);
    
    complex* dx2 = newarray_atomic(complex,n);
    multiply_complex_array_scalar(n,dx1,one_half*(*dt)); 
    add_to_complex_array(n,xt,dx1); // x0+.5dx1*dt
    xt[n] += one_half*(*dt); // t0+.5dt
    Ht->evaluate(n+1, xt, RHS);
    negate_complex_array(n,RHS);
    Hx->evaluate(n+1, xt, LHS);
    solve_via_lapack(n,LHS,1,RHS,dx2);
    
    complex* dx3 = newarray_atomic(complex,n);
    multiply_complex_array_scalar(n,dx2,one_half*(*dt));
    copy_complex_array(n,x0t0,xt); // spare t
    add_to_complex_array(n,xt,dx2); // x0+.5dx2*dt
    // xt[n] += one_half*(*dt); // t0+.5dt (SAME)
    Ht->evaluate(n+1, xt, RHS);
    negate_complex_array(n,RHS);
    Hx->evaluate(n+1, xt, LHS);
    solve_via_lapack(n,LHS,1,RHS,dx3);
    
    complex* dx4 = newarray_atomic(complex,n);
    multiply_complex_array_scalar(n,dx3,*dt);
    copy_complex_array(n+1,x0t0,xt);
    add_to_complex_array(n,xt,dx3); // x0+dx3*dt
    xt[n] += *dt; // t0+dt
    Ht->evaluate(n+1, xt, RHS);
    negate_complex_array(n,RHS);
    Hx->evaluate(n+1, xt, LHS);
    solve_via_lapack(n,LHS,1,RHS,dx4);
    
    // "dx1" = .5*dx1*dt, "dx2" = .5*dx2*dt, "dx3" = dx3*dt
    multiply_complex_array_scalar(n,dx4,*dt);
    multiply_complex_array_scalar(n,dx1,2);
    multiply_complex_array_scalar(n,dx2,4);
    multiply_complex_array_scalar(n,dx3,2);
    add_to_complex_array(n,dx4,dx1);
    add_to_complex_array(n,dx4,dx2);
    add_to_complex_array(n,dx4,dx3);
    multiply_complex_array_scalar(n,dx4,1.0/6);
    copy_complex_array(n,dx4,dx);
    deletearray(dx1);
    deletearray(dx2);
    deletearray(dx3);
    deletearray(dx4);
  } break;
  default: ERROR("unknown predictor"); 
  };
  deletearray(LHS);
  deletearray(RHS);
}


void StraightLineProgram::corrector()
{
  int n = num_inputs - 2; // n = size of vectors and matrices 
  double epsilon2 = 1e-10; // square of the tolerance
  double theSmallestNumber = 1e-12; 
  double minStep = 1e-6;

  complex* x0t = nodes+num_consts;
  complex* t = x0t+n; 
  complex* dt = t+1;
  StraightLineProgram* Hx = catalog[program->array[5]];
  StraightLineProgram* H = catalog[program->array[6]];
  int maxCorSteps = program->array[7]; 
  if (1-t->getreal()<theSmallestNumber && dt->getreal()<=minStep) 
    maxCorSteps = program->array[8]; // finalMaxCorSteps 
  
  complex* x1 = nodes+num_consts+num_inputs; // output
  complex* dx = x1+n; // on return: estimate of the error

  complex* x1t = newarray(complex, n+1); 
  copy_complex_array(n+1, x0t, x1t);
  complex* RHS = newarray_atomic(complex, n);
  complex* LHS = newarray_atomic(complex, n*n);
  int i=0; // number of steps
  do {
    i++;
    H->evaluate(n+1,x1t, RHS);
    negate_complex_array(n,RHS);
    Hx->evaluate(n+1,x1t, LHS);
    solve_via_lapack(n,LHS,1,RHS,dx);
    add_to_complex_array(n,x1t,dx);
  } while (norm2_complex_array(n,dx)>epsilon2*norm2_complex_array(n,x1t) and i<maxCorSteps);

  copy_complex_array(n,x1t,x1);

  deletearray(x1t);
  deletearray(LHS);
  deletearray(RHS);
}

////////////////// PathTracker //////////////////////////////////////////////////////////////////////////////////////////////

int PathTracker::num_path_trackers = 0;
PathTracker* PathTracker::catalog[MAX_NUM_PATH_TRACKERS];

PathTracker::PathTracker()
{
  raw_solutions = NULL;
  solutions = NULL;
}

PathTracker::~PathTracker()
{ 
  for (int i=0; i<n_sols; i++)
    raw_solutions[i].release();
  deletearray(raw_solutions); 
}

// a function that creates a PathTracker object, builds the homotopy, slps for predictor and corrector given a target system
// input: a (1-row) matrix of polynomials 
// out: the number of PathTracker
PathTracker_OrNull* PathTracker::make(Matrix *HH) 
{
  /* 
  if (HH->n_rows()!=1) { 
    ERROR("1-row matrix expected");
    return NULL;
  };
  */

  PathTracker *p = new PathTracker;
  const PolyRing* R = p->homotopy_R = HH->get_ring()->cast_to_PolyRing();
  if (R==NULL) {
    ERROR("polynomial ring expected");
    return NULL;
  }
  const Ring* K = R->getCoefficients();
  if (K->is_CCC())
    p->C = K->cast_to_CCC();
  else {
    ERROR("complex coefficients expected");
    return NULL;
  }

  p->H = HH;
  p->slpH = NULL;
  for (int i=0; i<HH->n_cols(); i++) {
    StraightLineProgram* slp = StraightLineProgram::make(R, HH->elem(0,i));
    if (p->slpH == NULL) p->slpH = slp;
    else {
      StraightLineProgram* t = p->slpH->concatenate(slp);
      delete slp;
      delete p->slpH;
      p->slpH = t;
    }
  }
  p->slpHxt = p->slpH->jacobian(true, p->slpHxH, true, p->slpHxtH);
  return p;
}

// for SLPs constructed on the top level
PathTracker_OrNull* PathTracker::make(StraightLineProgram* slp_pred, StraightLineProgram* slp_corr)
{
  PathTracker *p = new PathTracker;
  p->H = NULL;
  p->slpH = NULL;
  p->slpHxt = p->slpHxtH = slp_pred;
  p->slpHxH = slp_corr;
  p->C = NULL;
  return p;
}

int PathTracker::makeFromHomotopy(Matrix *HH) 
{
  if (num_path_trackers>MAX_NUM_PATH_TRACKERS) {
    ERROR("max number of path trackers exceeded");
    return -1;
  };
  PathTracker* p = catalog[num_path_trackers] = new PathTracker;
  p->number = num_path_trackers++;
  if (HH->n_rows()!=1) { 
    ERROR("1-row matrix expected");
    return -1;
  };
  return p->number;
}
  



void rawSetParametersPT(PathTracker* PT, M2_bool is_projective,
			M2_RRR init_dt, M2_RRR min_dt, 
			M2_RRR dt_increase_factor, M2_RRR dt_decrease_factor, int num_successes_before_increase,
			M2_RRR epsilon, int max_corr_steps, M2_RRR end_zone_factor, M2_RRR infinity_threshold,
			int pred_type)
{
  PT->is_projective = is_projective;
  PT->init_dt = init_dt;
  PT->min_dt = min_dt;
  PT->dt_increase_factor = dt_increase_factor; 
  PT->dt_decrease_factor = dt_decrease_factor;
  PT->num_successes_before_increase = num_successes_before_increase;
  PT->epsilon = epsilon;
  PT->max_corr_steps = max_corr_steps;
  PT->end_zone_factor = end_zone_factor;
  PT->infinity_threshold = infinity_threshold;
  PT->pred_type = pred_type;
}

void rawLaunchPT(PathTracker* PT, const Matrix* start_sols)
{
  PT->track(start_sols);
}

const MatrixOrNull *rawGetSolutionPT(PathTracker* PT, int solN)
{
  return PT->getSolution(solN);
}

const MatrixOrNull *rawGetAllSolutionsPT(PathTracker* PT)
{
  return PT->getAllSolutions();
}

int rawGetSolutionStatusPT(PathTracker* PT, int solN)
{
  return PT->getSolutionStatus(solN);
}

int rawGetSolutionStepsPT(PathTracker* PT, int solN)
{
  return PT->getSolutionSteps(solN);
}

M2_RRRorNull rawGetSolutionLastTvaluePT(PathTracker* PT, int solN)
{
  return PT->getSolutionLastT(solN);
}

M2_RRRorNull rawGetSolutionRcondPT(PathTracker* PT, int solN)
{
  return PT->getSolutionRcond(solN);
}

const MatrixOrNull *rawRefinePT(PathTracker* PT, const Matrix* sols, M2_RRR tolerance, int max_corr_steps_refine)
{
  return PT->refine(sols, tolerance, max_corr_steps_refine);
}

int PathTracker::track(const Matrix* start_sols)
{
  double the_smallest_number = 1e-13;
  double epsilon2 = mpfr_get_d(epsilon,GMP_RNDN); epsilon2 *= epsilon2; //epsilon^2
  double t_step = mpfr_get_d(init_dt,GMP_RNDN); // initial step
  double dt_min_dbl = mpfr_get_d(min_dt,GMP_RNDN);
  double dt_increase_factor_dbl = mpfr_get_d(dt_increase_factor,GMP_RNDN);
  double dt_decrease_factor_dbl = mpfr_get_d(dt_decrease_factor,GMP_RNDN);
  double infinity_threshold2 = mpfr_get_d(infinity_threshold,GMP_RNDN); infinity_threshold2 *= infinity_threshold2;
  double end_zone_factor_dbl = mpfr_get_d(end_zone_factor,GMP_RNDN);
  
  if (C==NULL) C = start_sols->get_ring()->cast_to_CCC(); //fixes the problem for PrecookedSLPs

  int n = n_coords = start_sols->n_cols();  
  n_sols = start_sols->n_rows();

  if (gbTrace>1) printf("epsilon2 = %e, t_step = %lf, dt_min_dbl = %lf, dt_increase_factor_dbl = %lf, dt_decrease_factor_dbl = %lf\n", 
			epsilon2, t_step, dt_min_dbl, dt_increase_factor_dbl, dt_decrease_factor_dbl);

  // memory distribution for arrays
  complex* s_sols = newarray(complex,n*n_sols);
  raw_solutions = newarray(Solution,n_sols);
  complex* x0t0 = newarray(complex,n+1); 
    complex* x0 =  x0t0;
    complex* t0 = x0t0+n;
  complex* x1t1 = newarray(complex,n+1); 
  //  complex* x1 =  x1t1;
  //  complex* t1 = x1t1+n;
  complex* dxdt = newarray(complex,n+1); 
    complex* dx =  dxdt;
    complex* dt = dxdt+n;
  complex* Hxt = newarray_atomic(complex, n*(n+1));
  complex* HxtH = newarray_atomic(complex, n*(n+2));
  complex* HxH = newarray_atomic(complex, n*(n+1));
    complex *LHS, *RHS;
  complex one_half(0.5,0);
  complex* xt = newarray_atomic(complex,n+1);
  complex* dx1 = newarray_atomic(complex,n);
  complex* dx2 = newarray_atomic(complex,n);
  complex* dx3 = newarray_atomic(complex,n);
  complex* dx4 = newarray_atomic(complex,n);

  // read solutions: rows are solutions
  int i,j;
  complex* c = s_sols;
  for(i=0; i<n_sols; i++) 
    for(j=0; j<n; j++,c++) 
      *c = complex(BIGCC_VAL(start_sols->elem(i,j)));
				
  Solution* t_s = raw_solutions; //current target solution
  complex* s_s = s_sols; //current start solution
  				
  for(int sol_n =0; sol_n<n_sols; sol_n++, s_s+=n, t_s++) {
    t_s->make(n,s_s); // cook a Solution
    t_s->status = PROCESSING;
    bool end_zone = false;
    double tol2 = epsilon2; // current tolerance squared, will change in end zone
    copy_complex_array(n,s_s,x0);
    *t0 = complex(0,0);

    *dt = complex(t_step);
    int predictor_successes = 0;
    int count = 0; // number of steps
    while (t_s->status == PROCESSING && 1 - t0->getreal() > the_smallest_number) {
      if (1 - t0->getreal() <= end_zone_factor_dbl+the_smallest_number && !end_zone) {
	end_zone = true;
	// to do: see if this path coinsides with any other path on entry to the end zone
      }
      if (end_zone) { 
	  if ( dt->getreal() > 1 - t0->getreal() ) 
	  *dt = complex(1-t0->getreal()); 
	} 
      else { 
	  if ( dt->getreal() > 1 - end_zone_factor_dbl - t0->getreal() ) 
	  *dt = complex(1 - end_zone_factor_dbl - t0->getreal()); 
	}  
      
      //printf("p: dt = %lf\n", dt->getreal()); 
      
      // PREDICTOR in: x0t0,dt,pred_type
      //           out: dx
      switch(pred_type) {
      case TANGENT: {
	slpHxt->evaluate(n+1,x0t0, Hxt);
	LHS = Hxt; 	
	RHS = Hxt+n*n; 
	multiply_complex_array_scalar(n,RHS,-*dt);
	solve_via_lapack_without_transposition(n,LHS,1,RHS,dx);
      } break;
      case EULER: {
	slpHxtH->evaluate(n+1,x0t0, HxtH); // for Euler "H" is attached
        LHS = HxtH;
        RHS = HxtH+n*(n+1); // H
	complex* Ht = RHS-n; 
	multiply_complex_array_scalar(n,Ht,*dt);
	add_to_complex_array(n,RHS,Ht);
	negate_complex_array(n,RHS);
	solve_via_lapack_without_transposition(n,LHS,1,RHS,dx);
      } break;
      case RUNGE_KUTTA: {
	copy_complex_array(n+1,x0t0,xt);
	
	// dx1
	slpHxt->evaluate(n+1,xt, Hxt);
	LHS = Hxt; 	
	RHS = Hxt+n*n; 
	//
	negate_complex_array(n,RHS);
	solve_via_lapack_without_transposition(n,LHS,1,RHS,dx1);
	
	// dx2
	multiply_complex_array_scalar(n,dx1,one_half*(*dt)); 
	add_to_complex_array(n,xt,dx1); // x0+.5dx1*dt
	xt[n] += one_half*(*dt); // t0+.5dt
	//
	slpHxt->evaluate(n+1,xt, Hxt);
	LHS = Hxt; 	
	RHS = Hxt+n*n; 
	//
	negate_complex_array(n,RHS);
	solve_via_lapack_without_transposition(n,LHS,1,RHS,dx2);
    
	// dx3
	multiply_complex_array_scalar(n,dx2,one_half*(*dt));
	copy_complex_array(n,x0t0,xt); // spare t
	add_to_complex_array(n,xt,dx2); // x0+.5dx2*dt
	// xt[n] += one_half*(*dt); // t0+.5dt (SAME)
	//
	slpHxt->evaluate(n+1,xt, Hxt);
	LHS = Hxt; 	
	RHS = Hxt+n*n; 
	//
	negate_complex_array(n,RHS);
	solve_via_lapack_without_transposition(n,LHS,1,RHS,dx3);
    
	// dx4
	multiply_complex_array_scalar(n,dx3,*dt);
	copy_complex_array(n+1,x0t0,xt);
	add_to_complex_array(n,xt,dx3); // x0+dx3*dt
	xt[n] += *dt; // t0+dt
	//
	slpHxt->evaluate(n+1,xt, Hxt);
	LHS = Hxt; 	
	RHS = Hxt+n*n; 
	//
	negate_complex_array(n,RHS);
	solve_via_lapack_without_transposition(n,LHS,1,RHS,dx4);
    
	// "dx1" = .5*dx1*dt, "dx2" = .5*dx2*dt, "dx3" = dx3*dt
	multiply_complex_array_scalar(n,dx4,*dt);
	multiply_complex_array_scalar(n,dx1,2);
	multiply_complex_array_scalar(n,dx2,4);
	multiply_complex_array_scalar(n,dx3,2);
	add_to_complex_array(n,dx4,dx1);
	add_to_complex_array(n,dx4,dx2);
	add_to_complex_array(n,dx4,dx3);
	multiply_complex_array_scalar(n,dx4,1.0/6);
	copy_complex_array(n,dx4,dx);
      } break;
      default: ERROR("unknown predictor"); 
      };

      copy_complex_array(n+1,x0t0,x1t1);
      add_to_complex_array(n+1,x1t1,dxdt);

      // CORRECTOR
      int n_corr_steps=0; 
      bool is_successful;
      do {
	n_corr_steps++;
	//
	slpHxH->evaluate(n+1,x1t1, HxH);
	LHS = HxH; 	
	RHS = HxH+n*n; // i.e., H
	//
	negate_complex_array(n,RHS);
	solve_via_lapack_without_transposition(n,LHS,1,RHS,dx);
	add_to_complex_array(n,x1t1,dx);
	is_successful = norm2_complex_array(n,dx) < tol2*norm2_complex_array(n,x1t1);
	//printf("c: |dx|^2 = %lf\n", norm2_complex_array(n,dx));
      } while (!is_successful and n_corr_steps<max_corr_steps);
    
      if (!is_successful) { 
	// predictor failure 
	predictor_successes = 0;
	*dt = complex(dt_decrease_factor_dbl)*(*dt);
	if (dt->getreal() < dt_min_dbl)
	  t_s->status = MIN_STEP_FAILED;
      } else { 
	// predictor success
	predictor_successes = predictor_successes + 1;
	copy_complex_array(n+1, x1t1, x0t0);
	count++;
	if (is_successful && predictor_successes >= num_successes_before_increase) { 
	  predictor_successes = 0;
	  *dt  = complex(dt_increase_factor_dbl)*(*dt);	
	}
      }
      if (norm2_complex_array(n,x0) > infinity_threshold2)
	t_s->status = INFINITY_FAILED;
    }
    // record the solution
    copy_complex_array(n, x0, t_s->x);    
    t_s->t = t0->getreal();
    if (t_s->status == PROCESSING)
      t_s->status = REGULAR;
    slpHxH->evaluate(n+1,x0t0,HxH);
    cond_number_via_svd(n, HxH/*Hx*/, t_s->rcond);
    t_s->num_steps = count;
    if (gbTrace>0) {
      if (sol_n%50==0) printf("\n");
      switch (t_s->status) {
      case REGULAR: printf("."); break;
      case INFINITY_FAILED: printf("I"); break;
      case MIN_STEP_FAILED: printf("M"); break;
      default: printf("-"); 
      }
      fflush(stdout);
    }
  }
  if (gbTrace>0) printf("\n");
  

  // clear arrays
  // deletearray(t_sols); // do not delete (same as raw_solutions) 
  deletearray(s_sols);
  deletearray(x0t0);
  deletearray(x1t1);
  deletearray(dxdt);
  deletearray(xt);
  deletearray(dx1);
  deletearray(dx2);
  deletearray(dx3);
  deletearray(dx4);
  deletearray(Hxt);
  deletearray(HxtH);
  deletearray(HxH);

  return n_sols;
}

MatrixOrNull* PathTracker::refine(const Matrix *sols, M2_RRR tolerance, int max_corr_steps_refine)
{
  double epsilon2 = mpfr_get_d(tolerance,GMP_RNDN); epsilon2 *= epsilon2;
  int n = n_coords; 
  if (! sols->get_ring()->is_CCC()) {
    ERROR("complex coordinates expected");
    return NULL;
  }  
  if (sols->n_cols() != n) {
    ERROR("incorrect number of coordinates");
    return NULL;
  }  
  n_sols = sols->n_rows();

  // memory distribution for arrays
  complex* s_sols = newarray(complex,n*n_sols);
  complex* dx = newarray(complex,n); 
  complex* x1t1 = newarray(complex,n+1); 
    complex* x1 =  x1t1;
    complex* t1 = x1t1+n;
  complex* HxH = newarray_atomic(complex, n*(n+1));
    complex *LHS, *RHS;

  // read solutions: rows are solutions
  int i,j;
  complex* c = s_sols;
  for(i=0; i<n_sols; i++) 
    for(j=0; j<n; j++,c++) 
      *c = complex(BIGCC_VAL(sols->elem(i,j)));

  complex* s_s = s_sols; //current solution 
  for(int sol_n =0; sol_n<n_sols; sol_n++, s_s+=n) {
    copy_complex_array(n,s_s,x1);
    *t1 = complex(1,0);
    // CORRECTOR
    bool is_successful; 
    int n_corr_steps=0; 
    do {
      n_corr_steps++;
      //
      slpHxH->evaluate(n+1,x1t1, HxH);
      LHS = HxH; 	
      RHS = HxH+n*n; // i.e., H
      //
      negate_complex_array(n,RHS);
      solve_via_lapack_without_transposition(n,LHS,1,RHS,dx);
      add_to_complex_array(n,x1t1,dx);
      is_successful = norm2_complex_array(n,dx) < epsilon2*norm2_complex_array(n,x1t1);
    } while (!is_successful and n_corr_steps<max_corr_steps_refine);
    if (!is_successful) 
      printf("max number of corrector steps exceeded for solution %d", sol_n);
    copy_complex_array(n,x1,s_s);
  }
  
  // make the output matrix
  FreeModule* S = C->make_FreeModule(n); 
  FreeModule* T = C->make_FreeModule(n_sols);
  MatrixConstructor mat(T,S);
  mpfr_t re, im;
  mpfr_init(re); mpfr_init(im);
  c = s_sols;
  for(i=0; i<n_sols; i++) 
    for(j=0; j<n; j++,c++) {
      mpfr_set_d(re, c->getreal(), GMP_RNDN);
      mpfr_set_d(im, c->getimaginary(), GMP_RNDN);
      ring_elem e = C->from_BigReals(re,im);
      mat.set_entry(i,j,e);
    }
  mpfr_clear(re); mpfr_clear(im);

  // clear arrays
  deletearray(s_sols);
  deletearray(dx);
  deletearray(x1t1);
  deletearray(HxH);

  return mat.to_matrix();
}

MatrixOrNull* PathTracker::getSolution(int solN)
{
  if (solN<0 || solN>=n_sols) return NULL;
  // construct output 
  FreeModule* S = C->make_FreeModule(n_coords); 
  FreeModule* T = C->make_FreeModule(1);
  MatrixConstructor mat(T,S);
  mpfr_t re, im;
  mpfr_init(re); mpfr_init(im);
  Solution* s = raw_solutions+solN;
  complex* c = s->x;
  for(int j=0; j<n_coords; j++,c++) {
    mpfr_set_d(re, c->getreal(), GMP_RNDN);
    mpfr_set_d(im, c->getimaginary(), GMP_RNDN);
    ring_elem e = C->from_BigReals(re,im);
    mat.set_entry(0,j,e);
  }
  mpfr_clear(re); mpfr_clear(im);
  return mat.to_matrix();
}

MatrixOrNull* PathTracker::getAllSolutions()
{
  // construct output 
  FreeModule* S = C->make_FreeModule(n_coords); 
  FreeModule* T = C->make_FreeModule(n_sols);
  MatrixConstructor mat(T,S);
  mpfr_t re, im;
  mpfr_init(re); mpfr_init(im);
  Solution* s = raw_solutions;
  for(int i=0; i<n_sols; i++,s++) {
    complex* c = s->x;
    for(int j=0; j<n_coords; j++,c++) {
      mpfr_set_d(re, c->getreal(), GMP_RNDN);
      mpfr_set_d(im, c->getimaginary(), GMP_RNDN);
      ring_elem e = C->from_BigReals(re,im);
      mat.set_entry(i,j,e);
    }
  }
  mpfr_clear(re); mpfr_clear(im);
  return (solutions = mat.to_matrix());
}

int PathTracker::getSolutionStatus(int solN)
{
  if (solN<0 || solN>=n_sols) return -1;
  return raw_solutions[solN].status;
}

int PathTracker::getSolutionSteps(int solN)
{
  if (solN<0 || solN>=n_sols) return -1;
  return raw_solutions[solN].num_steps;
}


M2_RRRorNull PathTracker::getSolutionLastT(int solN)
{
  if (solN<0 || solN>=n_sols) return NULL;
  M2_RRR result = (M2_RRR)getmem(sizeof(__mpfr_struct));
  mpfr_init2(result, C->get_precision());
  mpfr_set_d(result, raw_solutions[solN].t, GMP_RNDN);  
  return result;
}

M2_RRRorNull PathTracker::getSolutionRcond(int solN)
{
  if (solN<0 || solN>=n_sols) return NULL;
  M2_RRR result = (M2_RRR)getmem(sizeof(__mpfr_struct));
  mpfr_init2(result, C->get_precision());
  mpfr_set_d(result, raw_solutions[solN].rcond, GMP_RNDN);  
  return result;
}

void PathTracker::text_out(buffer& o) const
{
  slpHxt->stats_out(o);
  slpHxH->stats_out(o);

  /* int n = slpHxH->num_inputs;
  char buf[1000];
  complex input[n], output[n*n];
  for(int i=0; i<n; i++) {
    Nterm* t = H->elem(1,i).get_poly();
    input[i] = complex(BIGCC_VAL(t->coeff));
  }
  
  slpHxH->evaluate(n, input, output);
  for(int i=0; i<n*n; i++) {
    output[i].sprint(buf);
    o << "HxH[" << i << "] = " << buf << newline;
  }
  slpHxt->evaluate(n, input, output);
  for(int i=0; i<n*n; i++) {
    output[i].sprint(buf);
    o << "Hxt[" << i << "] = " << buf << newline;
  }
  slpH->text_out(o);
  slpHxH->text_out(o);
  */

}
 
// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:

