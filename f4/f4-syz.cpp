//////////////////////////////////////////
// Syzygy methods of F4GB
// ... used only if(using_syz)
//////////////////////////////////////////

#include <ctime>

#include "f4.hpp"
#include "monsort.hpp"

static clock_t syz_clock_sort_columns = 0;

////////////////////////////////////////////////////////////////
// Append rows to the syzygy matrix
//
// Idea: module-monomial $m e_i$ is represented by pair 
//   (monom=m*lm(g_i), comp=i), 
// where either g_i = gens[i] if i<gens.size(), 
//       or     g_i = gb[i+gens.size()], otherwise.
// "monom=m*lm(g_i)" forces the already coded module-monomial 
// comparison function to impose 
// Schreyer order. Should be changed eventually.

void F4GB::syz_load_gen(int which)
{
  if (!using_syz) return;
  
  M->copy(gens[which]->f.monoms /*lead monom*/, syz_next_monom);  
  M->set_component(which, syz_next_monom); 

  packed_monomial m = syz_next_monom;
  int newcol = syz_new_column(m); // this inserts a new monomial in syzH
  
  row_elem syz_r;
  syz_r.monom = m;
  syz_r.elem = which; // here info is duplicated: which == M->get_component(m)
  syz_r.len = 1;
  syz_r.coeffs = F4Mem::coefficients.allocate(1);
  static_cast<int *>(syz_r.coeffs)[0] = 0; // this represents 1 in the coefficient field 
  syz_r.comps = F4Mem::components.allocate(1);
  static_cast<int *>(syz_r.comps)[0] = newcol;
  syz->rows.push_back(syz_r);
}

void F4GB::syz_load_row(packed_monomial monom, int which)
{
  if (!using_syz) return;
  
  M->unchecked_mult(monom, gb[which]->f.monoms /*lead monom*/, syz_next_monom);  
  M->set_component(which+gens.size(), syz_next_monom); 
  
  packed_monomial m = syz_next_monom;
  int newcol = syz_new_column(m); // this inserts a new monomial in syzH

  row_elem syz_r;
  syz_r.monom = m;
  syz_r.elem = M->get_component(m); // .elem is not used at the monoment 
  syz_r.len = 1;
  syz_r.coeffs = F4Mem::coefficients.allocate(1);
  static_cast<int *>(syz_r.coeffs)[0] = 0; // this represents 1 in the coefficient field 
  syz_r.comps = F4Mem::components.allocate(1);
  static_cast<int *>(syz_r.comps)[0] = newcol;
  syz->rows.push_back(syz_r);
}


/////////////////////////////////////////////////////
// DEBUG routines 
/////////////////////////////////////////////////////

void F4GB::show_syz_matrix()
{
  if (!using_syz) return;
  fprintf(stderr, "---- ---- ---- ---- ---- ---- ----\n");
  MutableMatrix *q = F4toM2Interface::to_M2_MutableMatrix(KK,syz,gens,gb);
  buffer o;
  q->text_out(o);
  emit(o.str());
  fprintf(stderr, "---- ---- ---- ---- ---- ---- ----\n");
}

////////////////////////////////////////////////
// initialization
/////////////////////////////////////////////////

void F4GB::clear_syz_matrix()
{
  if (!using_syz) return;

  // syz
  syz_next_col_to_process = 0;
  syz->rows.clear(); //!!! are coeffs and comps killed here? 
  syz->columns.clear();

  syzH.reset();
  syzB.reset();
  syz_next_monom = syzB.reserve(1+M->max_monomial_size());
  syz_next_monom++;
}

void F4GB::reset_syz_matrix()
{
  if (!using_syz) return;

  syz_next_col_to_process = 0;
  syz_next_monom = B.reserve(1+M->max_monomial_size());
  syz_next_monom++;
}



int F4GB::syz_new_column(packed_monomial m)
{
  packed_monomial mm;
  if (syzH.find_or_insert(m, mm)) {// this should not happen
    fprintf(stderr, "syz_new_column: monomial not expected in syzH\n");
    error();
  }
  // the above line insures that
  // m is a packed monomial (with component), 
  // unique via the hash table syzH, syzB.
  syzB.intern(1+M->monomial_size(m));
  syz_next_monom = syzB.reserve(1+M->max_monomial_size());
  syz_next_monom++;

  column_elem c;
  int next_column = syz->columns.size();
  m[-1] = next_column;
  c.monom = m;
  c.head = -2;
  syz->columns.push_back(c);
  return next_column;
}

void F4GB::syz_reorder_columns()
{
  if (!using_syz) return;

  // Same as reorder_columns(), but for syzygy matrix

  int nrows = syz->rows.size();
  int ncols = syz->columns.size();

  // sort the columns

  int *column_order = F4Mem::components.allocate(ncols);
  int *ord = F4Mem::components.allocate(ncols);

  clock_t begin_time = clock();
  for (int i=0; i<ncols; i++)
    {
      column_order[i] = i;
    }

  if (gbTrace >= 2)
    fprintf(stderr, "ncomparisons = ");

  ColumnsSorter C(M, syz);
  QuickSorter<ColumnsSorter>::sort(&C, column_order, ncols);

  if (gbTrace >= 2)
    fprintf(stderr, "%ld, ", C.ncomparisons());
  clock_t end_time = clock();
  syz_clock_sort_columns += end_time - begin_time;
  double nsecs = end_time - begin_time;
  nsecs /= CLOCKS_PER_SEC;
  if (gbTrace >= 2)
    fprintf(stderr, " time = %f\n", nsecs);

  for (int i=0; i<ncols; i++)
    {
      ord[column_order[i]] = i;
    }

  // Now move the columns into position
  coefficient_matrix::column_array newcols;
  newcols.reserve(ncols);
  for (int i=0; i<ncols; i++)
    {
      long newc = column_order[i];
      newcols.push_back(syz->columns[newc]);
    }

  // Now reset the components in each row
  for (int r=0; r<nrows; r++)
    {
      row_elem &row = syz->rows[r];
      for (int i=0; i<row.len; i++)
	{
	  int oldcol = row.comps[i];
	  int newcol = ord[oldcol];
	  row.comps[i] = newcol;
	}
    }

  std::swap(syz->columns, newcols);
  F4Mem::components.deallocate(column_order);
  F4Mem::components.deallocate(ord);
}

//////// SYZYGY MANIPULATIONS //////////////

///////////////////////////////////////////////////
// create dense vector from syz->row[i]
void F4GB::syz_dense_row_fill_from_sparse(int i)
{
  if (!using_syz) return;

  row_elem& r = syz->rows[i];
  KK->dense_row_fill_from_sparse(syz_row, r.len, r.coeffs, r.comps);
}
 
////////////////////////////////////////////////////////////
// record the reduction of current row 
//        with respect to row[pivot] 
//        ( <li> and <lj> are the leading terms 
//          of <i> and <j> )  
void F4GB::syzygy_row_record_reduction(int pivot, int li, int lj)
{
  if (!using_syz) return;

  row_elem& r = syz->rows[pivot];
  int *elems = static_cast<int *>(syz_row.coeffs);
  int *sparseelems = static_cast<int *>(r.coeffs);
  int *comps = static_cast<int *>(r.comps);

  int a;                               
  const CoefficientRingZZp* R = KK->get_coeff_ring();                             
  R->divide(a,li,lj); // a = li/lj 
  for (int i=0; i<r.len; i++)
    R->subtract_multiple(elems[*comps++], a, *sparseelems++);
}

void F4GB::syzygy_row_divide(int i, int c)
{
  if (!using_syz) return;

  const CoefficientRingZZp* R = KK->get_coeff_ring();
  row_elem &r = syz->rows[i];
  f4vec elems = static_cast<int *>(r.coeffs);
  for (int j=0; j<r.len; j++)
    R->divide(elems[j],elems[j],c);
}

void F4GB::syz_dense_row_to_sparse_row(row_elem& s)
{
  if (!using_syz) return;

  const CoefficientRingZZp* Kp = KK->get_coeff_ring();
  int &result_len = s.len;
  F4CoefficientArray &result_sparse = s.coeffs;
  int *&result_comps = s.comps;
  
  // let's be lazy!!!
  int first = 0;
  int last = syz->columns.size()-1; 
  
  int *elems = static_cast<int *>(syz_row.coeffs);
  int len = 0;
  for (int i=first; i<=last; i++)
    if (!Kp->is_zero(elems[i])) len++;
  int *in_sparse = F4Mem::coefficients.allocate(len);
  int *in_comps = F4Mem::components.allocate(len);
  result_len = len;
  result_sparse = in_sparse;
  result_comps = in_comps;
  for (int i=first; i<=last; i++)
    if (!Kp->is_zero(elems[i]))
      {
        *in_sparse++ = elems[i];
        *in_comps++ = i;
        Kp->set_zero(elems[i]);
      }
}
