// Copyright 1995-2004 Michael E. Stillman

#include "style.hpp"
#include "text_io.hpp"
#include "ring.hpp"
#include "matrix.hpp"
#include "comb.hpp"
#include "det.hpp"
#include "polyring.hpp"
#include "termideal.hpp"
#include "assprime.hpp"
#include "monideal.hpp"
#include "relem.hpp"
#include "freemod.hpp"
#include "ntuple.hpp"

#include "exptable.h"

#include <vector>

#include "matrixcon.hpp"
#include "random.hpp"

Matrix::Matrix(const FreeModule *rows0, 
	       const FreeModule *cols0,
	       const int *degree_shift0,
	       vector<vec,gc_alloc> & entries0)
{
  _rows = const_cast<FreeModule *>(rows0);
  _cols = const_cast<FreeModule *>(cols0);
  _degree_shift = const_cast<int *>(degree_shift0);
  for (int i=0; i<cols0->rank(); i++)
    _entries.append(entries0[i]);

  int z = 234123 + get_ring()->get_hash_value() * (7 * n_rows() + 157 * n_cols());
  if (z < 0) z = -z;
  make_immutable(z);
}


const MatrixOrNull * Matrix::make(const FreeModule *target,
				  int ncols,
				  const RingElement_array *M)
{
  // Checks to make:
  // each vector in V is over same ring.

  const Ring *R = target->get_ring();

  if (M != 0)
    for (unsigned int i=0; i<M->len; i++)
      {
	if (R != M->array[i]->get_ring())
	  {
	    ERROR("expected vectors in the same ring");
	    return 0;
	  }
      }

  MatrixConstructor mat(target, ncols);
  if (M != 0)
    {
      unsigned int next = 0;
      for (int r=0; r <target->rank(); r++)
	for (int c=0; c < ncols && next < M->len; c++)
	  {
	    mat.set_entry(r,c,M->array[next]->get_value());
	    next++;
	  }
      mat.compute_column_degrees();
    }
  return mat.to_matrix();
}
const MatrixOrNull * Matrix::make(const FreeModule *target,
				  const FreeModule *source,
				  const M2_arrayint deg,
				  const RingElement_array *M)
{
  const Ring *R = target->get_ring();
  if (source->get_ring() != R)
    {
      ERROR("expected free modules over the same ring");
      return 0;
    }
  if (R->degree_monoid()->n_vars() != static_cast<int>(deg->len))
    {
      ERROR("expected degree of matrix to have %d entries", 
	    R->degree_monoid()->n_vars());
      return 0;
    }

  if (M != 0)
    {
      for (unsigned int i=0; i<M->len; i++)
	{
	  if (R != M->array[i]->get_ring())
	    {
	      ERROR("expected vectors in the same ring");
	      return 0;
	    }
	}
    }

  MatrixConstructor mat(target, source, deg->array);

  if (M != 0)
    {
      unsigned int next = 0;
      for (int r=0; r <target->rank(); r++)
	{
	  for (int c=0; c < source->rank(); c++)
	    {
	      mat.set_entry(r,c,M->array[next]->get_value());
	      next++;
	      if (next >= M->len) break;
	    }
	}
    }
  return mat.to_matrix();
}

bool Matrix::make_sparse_vecs(MatrixConstructor &mat,
			       const FreeModule *target,
			       int ncols,
			       const M2_arrayint rows,
			       const M2_arrayint cols,
			       const RingElement_array *entries)
  // returns false if an error, true otherwise.
  // Places the elements into 'mat'.
{
  const Ring *R = target->get_ring();
  for (unsigned int i=0; i<entries->len; i++)
    {
      if (R != entries->array[i]->get_ring())
	{
	  ERROR("expected vectors in the same ring");
	  return false;
	}
    }
  if (rows->len != cols->len || rows->len != entries->len)
    {
      ERROR("sparse matrix creation: encountered different length arrays");
      return false;
    }
  for (int x=0; x < entries->len; x++)
    {
      int r = rows->array[x];
      int c = cols->array[x];
      if (r < 0 || r >= target->rank())
	{
	  ERROR("sparse matrix creation: row index out of range");
	  return false;
	}
      if (c < 0 || c >=ncols)
	{
	  ERROR("sparse matrix creation: column index out of range");
	  return false;
	}
    }

  for (int x=0; x<entries->len; x++)
    {
      int r = rows->array[x];
      int c = cols->array[x];
      mat.set_entry(r,c, entries->array[x]->get_value());
    }
  return true;
}

const MatrixOrNull * Matrix::make_sparse(const FreeModule *target,
					 int ncols,
					 const M2_arrayint rows,
					 const M2_arrayint cols,
					 const RingElement_array *entries)
{
  MatrixConstructor mat(target, ncols);
  if (!Matrix::make_sparse_vecs(mat, target, ncols, rows, cols, entries))
    return 0; // error message has already been sent
  mat.compute_column_degrees();
  return mat.to_matrix();
}

const MatrixOrNull * Matrix::make_sparse(const FreeModule *target,
					 const FreeModule *source,
					 const M2_arrayint deg,
					 const M2_arrayint rows,
					 const M2_arrayint cols,
					 const RingElement_array *entries)
{
  MatrixConstructor mat(target, source, deg->array);
  if (!Matrix::make_sparse_vecs(mat, target, source->rank(), rows, cols, entries))
    return 0; // error message has already been sent
  return mat.to_matrix();
}

const MatrixOrNull * Matrix::remake(const FreeModule *target,
				    const FreeModule *source,
				    const M2_arrayint deg) const
{ 
  if (n_rows() != target->rank() || n_cols() != source->rank())
    {
      ERROR("wrong number of rows or columns");
      return 0;
    }
  if (deg->len != degree_monoid()->n_vars())
    {
      ERROR("degree for matrix has the wrong length");
      return 0;
    }
  const Ring *R = get_ring();
  const Ring *Rtarget = target->get_ring();
  const Ring *Rsource = source->get_ring();
  if (R != Rtarget || Rtarget != Rsource)
    {
      ERROR("expected same ring");
      return 0;
    }

  MatrixConstructor mat(target, source, deg->array);
  for (int i=0; i<source->rank(); i++)
    mat.set_column(i, R->copy_vec(_entries[i]));
  return mat.to_matrix();
}

const MatrixOrNull * Matrix::remake(const FreeModule *target) const
{
  if (n_rows() != target->rank())
    {
      ERROR("wrong number of rows");
      return 0;
    }
  const Ring *R = get_ring();
  if (R != target->get_ring())
    {
      ERROR("expected same ring");
      return 0;
    }

  MatrixConstructor mat(target, n_cols());
  for (int i=0; i<n_cols(); i++)
    mat.set_column(i, R->copy_vec(_entries[i]));
  mat.compute_column_degrees();
  return mat.to_matrix();
}

const MatrixOrNull * Matrix::make(const MonomialIdeal * mi)
{
  const PolynomialRing *P = mi->get_ring()->cast_to_PolynomialRing();
  if (P == 0)
    {
      ERROR("expected a matrix over a polynomial ring");
      return 0;
    }
  const Monoid *M = P->Nmonoms();
  int *mon = M->make_one();

  MatrixConstructor mat(P->make_FreeModule(1), mi->length());
  int next = 0;
  for (Index<MonomialIdeal> i = mi->last(); i.valid(); i--)
    {
      M->from_varpower((*mi)[i]->monom().raw(), mon);
      ring_elem f = P->make_flat_term(P->Ncoeffs()->from_int(1), mon);
      mat.set_entry(0,next++,f);
    }
  M->remove(mon);

  mat.compute_column_degrees();
  return mat.to_matrix();
}

ring_elem Matrix::elem(int i, int j) const
{
  return get_ring()->get_entry(elem(j), i);
}

#if 0
bool Matrix::get_entry(int r, int c, ring_elem &a) const
  // This one returns false if (r,c) is out of range.
{
  if (error_row_bound(r)) return false;
  if (error_column_bound(c)) return false;
  if (!get_ring()->get_entry(_entries[c], r, a))
    a = get_ring()->zero();
  return true;
}

bool Matrix::get_nonzero_entry(int r, int c, ring_elem &result) const
{
  if (c < 0 || c >= n_cols()) return false;
  return get_ring()->get_entry(_entries[c],r,result);
}
#endif

bool Matrix::is_equal(const Matrix &m) const
{
  if (this == &m) return true;
  if (get_hash_value() != m.get_hash_value())
    return false; // Note that if one is immutable, 
                  // and the other is mutable, false will be returned.
  if (get_ring() != m.get_ring())
    return false;
  if (n_rows() != m.n_rows())
    return false;
  if (n_cols() != m.n_cols())
    return false;
  for (int i=0; i<n_cols(); i++)
    if (! get_ring()->is_equal(elem(i), m.elem(i))) 
      return false;
  return true;
}

bool Matrix::is_zero() const
{
  for (int i=0; i<n_cols(); i++)
    if (elem(i) != 0) return false;
  return true;
}

int Matrix::is_homogeneous() const
{
  if (!get_ring()->is_graded()) return 0;
  int *d = degree_monoid()->make_one();
  for (int i=0; i<n_cols(); i++)
    {
      if (elem(i) == 0) continue;
      if (! get_ring()->vec_is_homogeneous(rows(), elem(i)))
	{
	  degree_monoid()->remove(d);
	  return 0;
	}
 
      get_ring()->vec_degree(rows(), elem(i), d);
      degree_monoid()->divide(d, degree_shift(), d);
      if (0 != degree_monoid()->compare(d, cols()->degree(i)))
	{
	  degree_monoid()->remove(d);
	  return 0;
	}
    }
  degree_monoid()->remove(d);
  return 1;
}

Matrix *Matrix::homogenize(int v, const M2_arrayint wts) const
{
  MatrixConstructor mat(rows(), n_cols());
  for (int i=0; i<n_cols(); i++)
    mat.set_column(i, get_ring()->vec_homogenize(rows(), elem(i), v, wts));
  mat.compute_column_degrees();
  return mat.to_matrix();
}

Matrix *Matrix::zero(const FreeModule *F, const FreeModule *G)
{
  if (F->get_ring() != G->get_ring())
    {
      ERROR("free modules have different base rings");
      return 0;
    }
  MatrixConstructor mat(F,G->rank());
  mat.set_column_degrees(G);
  return mat.to_matrix();
}

Matrix *Matrix::identity(const FreeModule *F)
{
  const ring_elem one = F->get_ring()->one();
  MatrixConstructor mat(F,F->rank());
  mat.set_column_degrees(F);
  for (int i=0; i<F->rank(); i++)
    mat.set_entry(i,i,one);
  return mat.to_matrix();
}

Matrix *Matrix::operator+(const Matrix &m) const
{
  if (get_ring() != m.get_ring())
    {
      ERROR("matrices have different base rings");
      return 0;
    }
  if (rows()->rank() != m.rows()->rank()
      || cols()->rank() != m.cols()->rank())
    {
      ERROR("matrices have different shapes");
      return 0;
    }
  
  const Ring *R = get_ring();
  const FreeModule *F = rows();
  const FreeModule *G = cols();
  const int *deg;

  if (!rows()->is_equal(m.rows()))
    F = R->make_FreeModule(n_rows());
  
  if (!cols()->is_equal(m.cols()))
    G = R->make_FreeModule(n_cols());
  
  if (EQ == degree_monoid()->compare(degree_shift(), m.degree_shift()))
    deg = degree_shift();
  else
    deg = degree_monoid()->make_one();

  MatrixConstructor mat(F,G,deg);
  for (int i=0; i<n_cols(); i++)
    {
      vec v = R->copy_vec(elem(i));
      vec w = R->copy_vec(m[i]);
      R->add_vec_to(v,w);
      mat.set_column(i, v);
    }
  return mat.to_matrix();
}

Matrix *Matrix::operator-(const Matrix &m) const
{
  if (get_ring() != m.get_ring())
    {
      ERROR("matrices have different base rings");
      return 0;
    }
  if (rows()->rank() != m.rows()->rank()
      || cols()->rank() != m.cols()->rank())
    {
      ERROR("matrices have different shapes");
      return 0;
    }

  const Ring *R = get_ring();
  const FreeModule *F = rows();
  const FreeModule *G = cols();
  const int *deg;

  if (!rows()->is_equal(m.rows()))
    F = R->make_FreeModule(n_rows());
  
  if (!cols()->is_equal(m.cols()))
    G = R->make_FreeModule(n_cols());
  
  if (EQ == degree_monoid()->compare(degree_shift(), m.degree_shift()))
    deg = degree_shift();
  else
    deg = degree_monoid()->make_one();

  MatrixConstructor mat(F,G,deg);
  for (int i=0; i<n_cols(); i++)
    mat.set_column(i, R->subtract_vec(elem(i),m[i]));
  return mat.to_matrix();
}

Matrix *Matrix::operator-() const
{
  MatrixConstructor mat(rows(), cols(), degree_shift());
  for (int i=0; i<n_cols(); i++)
    mat.set_column(i, get_ring()->negate_vec(elem(i)));
  return mat.to_matrix();
}

MatrixOrNull *Matrix::sub_matrix(const M2_arrayint r, const M2_arrayint c) const
{
  const FreeModule *F = rows()->sub_space(r);
  const FreeModule *G = cols()->sub_space(c);
  if (F == NULL || G == NULL)
    return 0;

  int *trans = newarray(int,n_rows());
  for (int i=0; i<n_rows(); i++)
    trans[i] = -1;

  for (unsigned j=0; j<r->len; j++)
    if (r->array[j] >= 0 && r->array[j] < n_rows())
      trans[r->array[j]] = j;

  MatrixConstructor mat(F,G,degree_shift());
  for (unsigned int i=0; i<c->len; i++)
    {
      vec v = elem(c->array[i]);
      for ( ; v != NULL; v = v->next)
	if (trans[v->comp] != -1)
	  mat.set_entry(trans[v->comp], i, v->coeff);
    }
  deletearray(trans);
  return mat.to_matrix();
}

MatrixOrNull *Matrix::sub_matrix(const M2_arrayint c) const
{
  const FreeModule *G = cols()->sub_space(c);
  if (G == NULL)
    return 0;

  MatrixConstructor mat(rows(),G,degree_shift());
  for (unsigned int i=0; i<c->len; i++)
    mat.set_column(i, get_ring()->copy_vec(elem(c->array[i])));
  return mat.to_matrix();
}

MatrixOrNull *Matrix::reshape(const FreeModule *F, const FreeModule *G) const
  // Reshape 'this' : F <--- G, where 
  // (rank F)(rank G) = (nrows this)(ncols this)
{
  if (F->get_ring() != get_ring() || G->get_ring() != get_ring())
    {
      ERROR("reshape: expected same ring");
      return 0;
    }
  if (n_rows() * n_cols() != F->rank() * G->rank())
    {
      ERROR("reshape: ranks of freemodules incorrect");
      return 0;
    }

  // EFFICIENCY: might be better to sort columns at end?
  MatrixConstructor mat(F,G,0);
  for (int c=0; c<n_cols(); c++)
    for (vecterm *p = elem(c); p != NULL; p = p->next)
      {
	// Determine new component
	int loc = c * n_rows() + p->comp;
	int result_col = loc / F->rank();
	int result_row = loc % F->rank();

	mat.set_entry(result_row,result_col,p->coeff);
      }
  return mat.to_matrix();
}

MatrixOrNull *Matrix::flip(const FreeModule *F, const FreeModule *G)
{
  const Ring *R = F->get_ring();
  if (R != G->get_ring())
    {
      ERROR("flip: expected same ring");
      return 0;
    }
  const FreeModule *H = F->tensor(G);
  const FreeModule *K = G->tensor(F);

  MatrixConstructor mat(K,H,0);
  int next = 0;
  for (int f=0; f<F->rank(); f++)
    for (int g=0; g<G->rank(); g++)
      mat.set_column(next++, R->e_sub_i(f + g * F->rank()));
  return mat.to_matrix();
}

Matrix *Matrix::transpose() const
{
  const FreeModule *F = cols()->transpose();
  const FreeModule *G = rows()->transpose();
  int *deg = degree_monoid()->make_one();
  degree_monoid()->divide(deg, degree_shift(), deg);

  MatrixConstructor mat(F,G,deg);
  degree_monoid()->remove(deg);

  // The efficiency of this code relies on the way of ordering
  // the sparse vectors (lead term has largest component)
  for (int c=0; c<n_cols(); c++)
    {
      for (vec t = elem(c); t != 0; t=t->next)
	mat.set_entry(c,t->comp,t->coeff);
    }
  return mat.to_matrix();
}

Matrix *Matrix::scalar_mult(const ring_elem r, bool opposite_mult) const
{
  const Ring *R = get_ring();
  int *deg = degree_monoid()->make_one();
  if (!R->is_zero(r))
    R->degree(r, deg);
  degree_monoid()->mult(deg, degree_shift(), deg);
  MatrixConstructor mat(rows(), cols(), deg);
  for (int i=0; i<n_cols(); i++)
    {
      vec w = R->copy_vec(elem(i));
      R->mult_vec_to(w,r,opposite_mult);
      mat.set_column(i, w);
    }
  return mat.to_matrix();
}

Matrix *Matrix::concat(const Matrix &m) const
{
  if (get_ring() != m.get_ring())
    {
      ERROR("concat: different base rings");
      return 0;
    }
  if (n_rows() != m.n_rows())
    {
      ERROR("concat: matrices have different numbers of rows");
      return 0;
    }

  const FreeModule *G = cols()->direct_sum(m.cols());
  MatrixConstructor mat(rows(), G, 0);
  int i;
  int nc = n_cols();
  for (i=0; i<nc; i++)
    mat.set_column(i, get_ring()->copy_vec(elem(i)));
  for (i=0; i<m.n_cols(); i++)
    mat.set_column(nc+i, get_ring()->copy_vec(m.elem(i)));
  return mat.to_matrix();
}

Matrix *Matrix::direct_sum(const Matrix *m) const
{
  if (get_ring() != m->get_ring())
    {
      ERROR("concat: different base rings");
      return 0;
    }
  const int *deg;
  if (EQ == degree_monoid()->compare(degree_shift(), m->degree_shift()))
    deg = degree_shift();
  else
    deg = 0;

  const FreeModule *F = rows()->direct_sum(m->rows());
  const FreeModule *G = cols()->direct_sum(m->cols());

  MatrixConstructor mat(F,G, 0);

  int i;
  int nr = n_rows();
  int nc = n_cols();
  for (i=0; i<nc; i++) 
    mat.set_column(i, get_ring()->copy_vec(elem(i)));
  for (i=0; i<m->n_cols(); i++)
    mat.set_column(nc+i, get_ring()->component_shift(nr, m->elem(i)));
  return mat.to_matrix();
}

Matrix *Matrix::mult(const Matrix *m, bool opposite_mult) const
{
  const Ring *R = get_ring();
  if (R != m->get_ring())
    {
      ERROR("matrix mult: different base rings");
      return 0;
    }
  if (n_cols() != m->n_rows())
    {
      ERROR("matrix mult: matrix sizes don't match");
      return 0;
    }

  int *deg = degree_monoid()->make_new(degree_shift());
  degree_monoid()->mult(deg, m->degree_shift(), deg);

  MatrixConstructor mat(rows(), m->cols(), deg);

  degree_monoid()->remove(deg);

  for (int i=0; i<m->n_cols(); i++)
    mat.set_column(i, R->mult_vec_matrix(this, m->elem(i), opposite_mult));
  return mat.to_matrix();
}

Matrix *Matrix::module_tensor(const Matrix *m) const
{
  if (get_ring() != m->get_ring())
    {
      ERROR("module tensor: different base rings");
      return 0;
    }
  FreeModule *F = rows()->tensor(m->rows());
  FreeModule *G = rows()->tensor(m->cols());
  FreeModule *G1 = m->rows()->tensor(cols());
  G->direct_sum_to(G1);
  deleteitem(G1);

  MatrixConstructor mat(F,G, 0);

  int i, j, next=0;

  for (i=0; i<n_rows(); i++)
    for (j=0; j<m->n_cols(); j++)
      mat.set_column(next++, get_ring()->component_shift(i * m->n_rows(), m->elem(j)));

  for (i=0; i<m->n_rows(); i++)
    for (j=0; j<n_cols(); j++)
      mat.set_column(next++, get_ring()->tensor_shift(m->n_rows(), i, elem(j)));
  return mat.to_matrix();
}

#if 0
// REMOVE THIS ONE??
Matrix *Matrix::random(const Ring *R, int r, int c)
{
  FreeModule *F = R->make_FreeModule(r);
  FreeModule *G = R->make_FreeModule(c);
  Matrix *result = new Matrix(F,G);
  for (int i=0; i<c; i++)
    (*result)[i] = F->random();
  return result;
}
#endif

Matrix *Matrix::random(const Ring *R, 
		       int r, int c, 
		       double fraction_non_zero, 
		       int special_type) // 0: general, 1:upper triangular, others?
{
  bool doing_fraction = false;
  int threshold;

  FreeModule *F = R->make_FreeModule(r);
  FreeModule *G = R->make_FreeModule(c);
  MatrixConstructor mat(F,G, 0);

  // Loop through all selected elements, flip a 'fraction_non_zero' coin, and if non-zero
  // set that element.

#warning "fraction_non_zero not yet used"  
  mpz_t a;
  mpz_init(a);

  if (fraction_non_zero != 1.0)
    {
      doing_fraction = true;
      int f = static_cast<int>(fraction_non_zero * 10000);
      threshold = 10000 - f;
      printf("threshold is %d\n", threshold);
    }


  if (special_type == 0)
    {
      for (int i=0; i<c; i++)
	for (int j=0; j<r; j++)
	  {
	    if (doing_fraction)
	      {
		int32 u = Random::random0(10000);
		if (u > threshold) continue;
	      }
	    Random::random_integer(a);
	    mat.set_entry(j,i,R->from_int(a));
	  }
    }
  else if (special_type == 1)
    {
      for (int i=0; i<c; i++)
	{
	  int top = (i>=r ? r : i);
	  for (int j=0; j<top; j++)
	    {
	      if (doing_fraction)
		{
		  int32 u = Random::random0(10000);
		  if (u > threshold) continue;
		}
	      Random::random_integer(a);
	      mat.set_entry(j,i,R->from_int(a));
	    }
	}

    }
  mpz_clear(a);
  return mat.to_matrix();
}


Matrix *Matrix::tensor(const Matrix *m) const
{
  if (get_ring() != m->get_ring())
    {
      ERROR("matrix tensor: different base rings");
      return 0;
    }

  const FreeModule *F = rows()->tensor(m->rows());
  const FreeModule *G = cols()->tensor(m->cols());
  int *deg = degree_monoid()->make_new(degree_shift());
  degree_monoid()->mult(deg, m->degree_shift(), deg);

  MatrixConstructor mat(F,G,deg);
  degree_monoid()->remove(deg);
  int i, j, next = 0;
  for (i=0; i<n_cols(); i++)
    for (j=0; j<m->n_cols(); j++)
      mat.set_column(next++, get_ring()->tensor(rows(), elem(i), 
						m->rows(), (*m)[j]));
  return mat.to_matrix();
}

Matrix *Matrix::diff(const Matrix *m, int use_coef) const
{
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  if (P == 0) 
    {
      ERROR("expected a polynomial ring");
      return 0;
    }
  if (P != m->get_ring())
    {
      ERROR("matrix diff: different base rings");
      return 0;
    }
  FreeModule *F1 = rows()->transpose();
  const FreeModule *F = F1->tensor(m->rows());
  FreeModule *G1 = cols()->transpose();
  const FreeModule *G = G1->tensor(m->cols());
  int *deg = degree_monoid()->make_one();
  degree_monoid()->divide(m->degree_shift(), degree_shift(), deg);
  deleteitem(F1);
  deleteitem(G1);

  MatrixConstructor mat(F,G,deg);
  degree_monoid()->remove(deg);
  int i, j, next=0;
  for (i=0; i<n_cols(); i++)
    for (j=0; j<m->n_cols(); j++)
      mat.set_column(next++, P->vec_diff(elem(i), m->rows()->rank(), m->elem(j), use_coef));
  return mat.to_matrix();
}

Matrix *Matrix::lead_term(int nparts) const
    // Select those monomials in each column
    // which are maximal in the order under
    // the first n parts of the monomial order.
{
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  MatrixConstructor mat(rows(),cols(),degree_shift());

  for (int i=0; i<n_cols(); i++)
    mat.set_column(i, P->vec_lead_term(nparts, rows(), elem(i)));
  return mat.to_matrix();
}

#if 0
void Matrix::minimal_lead_terms_ZZ(intarray &result) const
{
  int x;
  M2_arrayint indices;
  array<TermIdeal *> mis;
  const array<vec> vecs = _entries;
  indices = rows()->sort(vecs, NULL, 0, 1);
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  const FreeModule *Rsyz = P->get_Rsyz(); // NULL if not a quotient ring.
  FreeModule *Gsyz = P->make_FreeModule(vecs.length());
  for (x=0; x<n_cols(); x++)
    mis.append(new TermIdeal(P,Gsyz));
  for (int i=0; i<vecs.length(); i++)
    {
      vec v = vecs[indices->array[i]];
      vec gsyz, rsyz;
      if (v == NULL) continue;
      if (TI_TERM != mis[v->comp]->search(v->coeff, v->monom, gsyz, rsyz))
	{
	  mis[v->comp]->insert_minimal(
				       new tagged_term(P->Ncoeffs()->copy(v->coeff),
						       P->Nmonoms()->make_new(v->monom),
						       NULL,
						       NULL));
	  result.append(indices->array[i]);
	}
      Gsyz->remove(gsyz);
      if (rsyz != NULL) Rsyz->remove(rsyz);
    }
  for (x=0; x<n_cols(); x++)
    deleteitem(mis[x]);
}
#endif
#if 0
// OLDER THAN THE PREVIOUS VERSION!!
Matrix Matrix::minimal_lead_terms_ZZ() const
{
  int x;
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  FreeModule *Gsyz = P->make_FreeModule(n_cols());
  bump_up(Gsyz);
  array< queue<tagged_term *> > allterms;
  for (int i=0; i<n_cols(); i++)
    {
      vec v = elem(i);
      if (v == NULL) continue;
      allterms[v->comp].insert(
			       new tagged_term(P->Ncoeffs()->copy(v->coeff),
					       P->Nmonoms()->make_new(v->monom),
					       Gsyz->e_sub_i(i),
					       NULL));
    }
  Matrix result(rows());
  for (x=0; x<n_cols(); x++)
    {
      if (allterms[x].length() > 0)
	{
	  TermIdeal *ti = TermIdeal::make_termideal(P,Gsyz,allterms[x]);
	  // Loop through and add the corresponding elements in...
	  for (cursor_TermIdeal k(ti); k.valid(); ++k)
	    {
	      tagged_term *t = *k;
	      vec gsyz = t->_gsyz;
	      vec v = NULL;
	      rows()->apply_map(v, gsyz, entries);
	      rows()->negate_to(v);
	      result.append(v);
	    }
	  deleteitem(ti);
	}
    }
  bump_down(Gsyz);
  return result;
}
#endif

#if 0
void Matrix::minimal_lead_terms(intarray &result) const
{
  if (get_ring()->Ncoeffs()->is_ZZ())
    {
      minimal_lead_terms_ZZ(result);
      return;
    }
  M2_arrayint indices;
  intarray vp;
  array<MonomialIdeal *> mis;
  const array<vec> vecs = _entries;
  indices = rows()->sort(vecs, NULL, 0, 1);
  for (int x=0; x<n_rows(); x++)
    mis.append(new MonomialIdeal(get_ring()));
  for (int i=0; i<vecs.length(); i++)
    {
      vec v = vecs[indices->array[i]];
      if (v == NULL) continue;
      // Reduce each one in turn, and replace.
      Bag *junk_bag;
      vp.shrink(0);
      rows()->lead_varpower(v, vp);
      if (!mis[v->comp]->search(vp.raw(),junk_bag))
	{
	  Bag *b = new Bag(indices->array[i], vp);
	  mis[v->comp]->insert(b);
	  result.append(indices->array[i]);
	}
    }
}

#endif
Matrix *Matrix::top_coefficients(Matrix *&monoms) const
{
  const PolynomialRing *R = get_ring()->cast_to_PolynomialRing();
  MatrixConstructor result(rows(), 0);
  MatrixConstructor cons_monoms(R->make_FreeModule(1), 0);
  for (int i=0; i<n_cols(); i++)
    {
      int var, exp;
      vec u = elem(i);
      vec v = R->vec_top_coefficient(u, var, exp);
      result.append(v);
      ring_elem a = R->var(var,exp);
      vec w = R->make_vec(0,a);
      cons_monoms.append(w);
    }
  monoms = cons_monoms.to_matrix();
  return result.to_matrix();
}

M2_arrayint_OrNull Matrix::elim_vars(int nparts) const
{
  intarray keep;
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  if (P == 0)
    {
      ERROR("expected polynomial ring");
      return 0;
    }
  int nslots = P->getMonoid()->n_slots(nparts);
  for (int i=0; i<n_cols(); i++)
    if (P->vec_in_subring(nslots, elem(i)))
      keep.append(i);
  M2_arrayint result = makearrayint(keep.length());
  for (unsigned int i=0; i<result->len; i++)
    result->array[i] = keep[i];
  return result;
}

M2_arrayint_OrNull Matrix::elim_keep(int nparts) const
{
  intarray keep;
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  if (P == 0)
    {
      ERROR("expected polynomial ring");
      return 0;
    }
  int nslots = P->getMonoid()->n_slots(nparts);
  for (int i=0; i<n_cols(); i++)
    if (!P->vec_in_subring(nslots, elem(i)))
      keep.append(i);
  M2_arrayint result = makearrayint(keep.length());
  for (unsigned int i=0; i<result->len; i++)
    result->array[i] = keep[i];
  return result;
}

Matrix *Matrix::divide_by_var(int n, int maxd, int &maxdivided) const
{
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  if (P == 0)
    {
      ERROR("expected polynomial ring");
      return 0;
    }
  MatrixConstructor mat(rows(), 0);
  maxdivided = 0;
  for (int i=0; i<n_cols(); i++)
    {
      if (elem(i) != NULL)
	{
	  int lo,hi;
	  P->vec_degree_of_var(n, elem(i), lo,hi);
	  if (maxd >= 0 && lo > maxd)
	    lo = maxd;
	  if (lo > maxdivided)
	    maxdivided = lo;
	  mat.append(P->vec_divide_by_var(n, lo, elem(i)));
	}
    }
  return mat.to_matrix();
}

// ideal operations
MatrixOrNull *Matrix::koszul(int p) const
{
  if (n_rows() != 1)
    {
      ERROR("expected a matrix with one row");
      return 0;
    }
  
  FreeModule *F = cols()->exterior(p-1);
  FreeModule *G = cols()->exterior(p);
  const Ring *R = get_ring();
  MatrixConstructor mat(F,G,degree_shift());
  if (p <= 0 || p > n_cols()) return mat.to_matrix();
  int *a = newarray(int,p);
  for (int c=0; c < G->rank(); c++)
    {
      comb::decode(c, a,p);
      int negate = ((p % 2) != 0);
      for (int r=p-1; r>=0; r--)
	{
	  negate = !negate;
	  swap(a[p-1], a[r]);
	  int x = comb::encode(a, p-1);
	  ring_elem f = elem(0, a[p-1]);
	  if (negate)
	    R->negate_to(f);

	  mat.set_entry(x,c,f);
	}
    }
  deletearray(a);
  return mat.to_matrix();
}

static int signdivide(int n, const int *a, const int *b, int *exp)
{
  int sign = 0;
  int sum = 0;
  for (int i=0; i<n; i++)
    {
      int e = a[i] - b[i];
      if (e < 0) return 0;
      exp[i] = e;
      sign += sum*e;
      sum += b[i];
    }
  sign %= 2;
  if (sign == 0) return 1;
  return -1;
}
MatrixOrNull *Matrix::koszul(const Matrix *r, const Matrix *c)
{
  // First check rings: r,c,'this' should be row vectors.
  // and the ring should be a polynomial ring
  const FreeModule *F = r->cols();


  const PolynomialRing *P = F->get_ring()->cast_to_PolynomialRing();
  if (P == NULL) return 0;
  const Ring *K = P->getLogicalCoefficients();
  const Monoid *M = P->getLogicalMonoid();

  MatrixConstructor mat(F, c->cols(), 0);

  int nvars = M->n_vars();
  int nrows = r->n_cols();
  int ncols = c->n_cols();
  int *aexp = newarray(int,nvars);
  int *bexp = newarray(int,nvars);
  int *result_exp = newarray(int,nvars);
  for (int i=0; i<ncols; i++)
    {
      if (c->elem(i) == 0) continue;
      const int *a = P->lead_logical_monomial(c->elem(i)->coeff);
      M->to_expvector(a, aexp);
      for (int j=0; j<nrows; j++)
	{
	  if (r->elem(j) == 0) continue;
	  const int *b = P->lead_logical_monomial(r->elem(j)->coeff);
	  M->to_expvector(b, bexp);
	  int sign = signdivide(nvars, aexp, bexp, result_exp);
	  if (sign != 0)
	    {
	      const int *m;
#warning "does m need to be initialized?"
	      M->from_expvector(m, result_exp);
	      ring_elem s = (sign > 0 ? K->one() : K->minus_one());
	      ring_elem f = P->make_logical_term(s,m);
	      mat.set_entry(j,i,f);
	    }
	}
    }
  deletearray(aexp);
  deletearray(bexp);
  deletearray(result_exp);
  return mat.to_matrix();
}

Matrix *Matrix::wedge_product(int p, int q, const FreeModule *F)
{
  const FreeModule *Fp = F->exterior(p);
  const FreeModule *Fq = F->exterior(q);
  const FreeModule *Fn = F->exterior(p+q);
  const FreeModule *G = Fp->tensor(Fq);
  const Ring *R = F->get_ring();

  MatrixConstructor mat(Fn,G, 0);

  if (p < 0 || q < 0 || p+q >F->rank())
    return mat.to_matrix();

  if (p == 0 || q == 0)
    {
      for (int i=0; i<G->rank(); i++)
	mat.set_entry(i,i,R->one());
      return mat.to_matrix();
    }

  int *a = newarray(int,p);
  int *b = newarray(int,q);
  int *c = newarray(int,p+q);
  int col = 0;

  for (int i=0; i<Fp->rank(); i++)
    {
      comb::decode(i, a, p);
      for (int j=0; j<Fq->rank(); j++)
	{
	  comb::decode(j, b, q);
	  int sgn = comb::mult_subsets(p,a,q,b,c);
	  if (sgn == 0)
	    {
	      col++;
	      continue;
	    }
	  ring_elem r = F->get_ring()->from_int(sgn);
	  int row = comb::encode(c,p+q);
	  mat.set_entry(row,col++,r);
	}
    }

  deletearray(a);
  deletearray(b);
  deletearray(c);
  return mat.to_matrix();
}

void Matrix::text_out(buffer &o) const
{
  int nrows = n_rows();
  int ncols = n_cols();

  buffer *p = newarray(buffer,nrows);
  //  buffer *p = new buffer[nrows];
  int r;
  for (int c=0; c<ncols; c++)
    {
      int maxcount = 0;
      for (r=0; r<nrows; r++)
	{
	  ring_elem f = elem(r,c);
	  get_ring()->elem_text_out(p[r], f);
	  get_ring()->remove(f);
	  if (p[r].size() > maxcount)
	    maxcount = p[r].size();
	}
      for (r=0; r<nrows; r++)
	for (int k=maxcount+1-p[r].size(); k > 0; k--)
	  p[r] << ' ';
    }
  for (r=0; r<nrows; r++)
    {
      p[r] << '\0';
      char *s = p[r].str();
      o << s << newline;
    }
  deletearray(p);
}

Matrix *Matrix::compress() const
{
  MatrixConstructor result(rows(), 0);
  for (int i=0; i<n_cols(); i++)
    if (elem(i) != 0)
      result.append(elem(i), cols()->degree(i));
  return result.to_matrix();
}


#if 0
int Matrix::moneq(const int *exp, int *m, const int *vars, int *exp2) const
    // Internal private routine for 'coeffs'.
    // exp2 is a scratch value.  It is a paramter so we only have to allocate 
    // it once...
{
  get_ring()->Nmonoms()->to_expvector(m, exp2);
  int nvars = get_ring()->n_vars();
  for (int i=0; i<nvars; i++)
    {
      if (vars[i] == 0) continue;
      if (exp[i] != exp2[i]) 
	return 0;
      else 
	exp2[i] = 0;
    }
  get_ring()->Nmonoms()->from_expvector(exp2, m);
  return 1;
}
vec Matrix::strip_vector(vec &f, const int *vars, 
			      const FreeModule *F, vec &vmonom) const
    // private routine for 'coeffs'
{
  if (f == NULL) 
    {
      vmonom = NULL;
      return NULL;
    }
  if (get_ring()->Nmonoms() == NULL)
    {
      vmonom = F->e_sub_i(0);
      vec result = f;
      f = NULL;
      return result;
    }
  // At this point, we know that we have a polynomial ring
  int nvars = get_ring()->n_vars();
  int *exp = newarray(int,nvars);
  int *scratch_exp = newarray(int,nvars);
  const Monoid *M = get_ring()->Nmonoms();

  M->to_expvector(f->monom, exp);
  for (int i=0; i<nvars; i++)
    if (vars[i] == 0) exp[i] = 0;

  // the following two lines do NOT work if 'F' is a Schreyer free module,
  // but this routine is private to 'coeffs', where this is not the case.
  vmonom = F->e_sub_i(0);
  M->from_expvector(exp, vmonom->monom);

  vecterm head;
  vecterm *newf = &head;
  vec result = NULL;

  // Loop through f: if monomial matches 'exp', strip and add to result,
  // otherwise leave alone, and place on head list.
  while (f != NULL)
    {
      if (moneq(exp, f->monom, vars, scratch_exp))
	{
	  vec temp = f;
	  f = f->next;
	  temp->next = NULL;
	  rows()->add_to(result, temp);
	}
      else
	{
	  newf->next = f;
	  f = f->next;
	  newf = newf->next;
	  newf->next = NULL;
	}
    }
  newf->next = NULL;
  f = head.next;

  deletearray(exp);
  deletearray(scratch_exp);
  return result;
}
#endif

Matrix *Matrix::remove_scalar_multiples() const
{
  bool keep;
  MatrixConstructor result(rows(), 0);
  for (int i=0; i<n_cols(); i++)
    {
      vec f = elem(i);
      if (f == NULL) continue;
      keep = true;
      for (int j=i+1; j<n_cols(); j++)
	{
	  vec g = elem(j);
	  if (g == NULL) continue;
	  if (get_ring()->vec_is_scalar_multiple(f, g))
	    {
	      keep = false;
	      break;
	    }
	}
      if (keep) result.append(get_ring()->copy_vec(f));
    }
  return result.to_matrix();
}

Matrix *Matrix::remove_monomial_factors(bool make_squarefree_only) const
// Divide each column v by the maximal monomial 'm' which divides v.
// If keep_one is true, divide by somewhat less, making the resulting monomial
// factor squarefree.
{
  MatrixConstructor result(rows(), 0);
  if (make_squarefree_only)
    {
      for (int i=0; i<n_cols(); i++)
	{
	  vec f = get_ring()->vec_monomial_squarefree(elem(i));
	  result.append(f);
	}
    }
  else
    {
      for (int i=0; i<n_cols(); i++)
	{
	  vec f = get_ring()->vec_remove_monomial_divisors(elem(i));
	  result.append(f);
	}
    }
  return result.to_matrix();
}

#if 0
// MES Aug 2002
Matrix *Matrix::simplify(int n) const
{
  int i,j, keep;
  Matrix *result = new Matrix(rows());

  switch (n) {
  case 1:
    for (i=0; i<n_cols(); i++)
      {
	vec f = elem(i);
	if (f == NULL) continue;
	result->append(rows()->copy(f));
      }
    break;
    //  case SIMP_SCALAR_MULTIPLES:
  case 2:
    for (i=0; i<n_cols(); i++)
      {
	vec f = elem(i);
	if (f == NULL) continue;
	keep = 1;
	for (j=i+1; j<n_cols(); j++)
	  {
	    vec g = elem(j);
	    if (g == NULL) continue;
	    if (rows()->is_scalar_multiple(f, g))
	      {
		keep = 0;
		break;
	      }
	  }
	if (keep) result->append(rows()->copy(f));
      }
    break;
  case 3:
    // Remove multiple monomial divisors (i.e. x^2*f --> x*f)
    for (i=0; i<n_cols(); i++)
      {
	vec f = elem(i);
	if (f == NULL) continue;
	result->append(rows()->monomial_squarefree(f));
      }
    break;
  case 4:
    // Remove monomial divisors (i.e. x*f --> f)
    for (i=0; i<n_cols(); i++)
      {
	vec f = elem(i);
	if (f == NULL) continue;
	result->append(rows()->remove_monomial_divisors(f));
      }
    break;
#if 0
  case SIMP_ZEROS:
    break;
  case SIMP_MULTIPLES:
    break;
  case SIMP_AUTO_REDUCE:
    break;
  case SIMP_SQUAREFREE:
    break;
  case SIMP_MONOMIAL_DIVISORS:
    break;
#endif
  default:
    ERROR("bad simplification type");
    return 0;
  }

  return result;
}
#endif
#if 0
// MES Aug 2002
Matrix *Matrix::auto_reduce() const
{
  array<vec> vecs;
  int i;
  for (i=0; i<n_cols(); i++)
    vecs.append(rows()->copy(elem(i)));
  rows()->auto_reduce(vecs);
  Matrix *result = new Matrix(rows());
  for (i=0; i<vecs.length(); i++)
    result->append(vecs[i]);
  return result;
}
#endif

#if 0
Matrix *Matrix::coeffs(const int *vars, Matrix * &result_monoms) const
{
  Matrix *result_coeffs = new Matrix(rows());
  result_monoms = new Matrix(get_ring()->make_FreeModule(1));	// One row matrix
  for (int j=0; j<n_cols(); j++)
    {
      vec f = rows()->copy(elem(j));
      vec vmonom;
      while (f != NULL)
	{
	  vec g = strip_vector(f, vars, result_monoms->rows(), vmonom);
	  result_coeffs->append(g);
	  result_monoms->append(vmonom);
	}
    }
  // MES: now sort both matrices...
  return result_coeffs;
}
#endif

MatrixOrNull *Matrix::monomials(M2_arrayint vars) const
  // Returns a one row matrix of all of the monomials in the variable subset 'vars'
  // which occur in 'this'.  These monomials are sorted into increasing degree order.
{
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  if (P == 0)
    {
      ERROR("expected a matrix over a polynomial ring");
      return 0;
    }
  const Monoid *M = P->getMonoid();
  const Ring *K = P->getCoefficients();
  int nvars = M->n_vars();
  // Check that 'vars' is valid
  for (unsigned int i=0; i<vars->len; i++)
    if (vars->array[i] < 0 || vars->array[i] >= nvars)
      {
	ERROR("expected a list of indices of indeterminates");
	return 0;
      }

  // Now collect all of the monomials
  int *mon = M->make_one();
  int *exp = newarray(int,M->n_vars());
  ring_elem one = K->from_int(1);
  exponent_table *E = exponent_table_new(50000, vars->len);

  for (int c=0; c<n_cols(); c++)
    {
      vec v = elem(c);
      for ( ; v != 0; v = v->next)
	{
	  for (Nterm *t = v->coeff; t != 0; t = t->next)
	    {
	      int *exp1 = newarray(int,vars->len);
	      M->to_expvector(t->monom, exp);
	      for (unsigned int i=0; i<vars->len; i++)
		exp1[i] = exp[vars->array[i]];
	      exponent_table_put(E, exp1, 1);
	    }
	}
    }

  // Take all of these monomials and make an array out of them
  MatrixConstructor mat(get_ring()->make_FreeModule(1),0);
  const void ** monoms = exponent_table_to_array(E);
  for (int i=0; i<nvars; i++) exp[i] = 0;
  for (int i=0; monoms[i] != 0; i += 2)
    {
      const int * exp1 = reinterpret_cast<const int *>(monoms[i]);
      for (unsigned int j=0; j<vars->len; j++)
	exp[vars->array[j]] = exp1[j];
      M->from_expvector(exp, mon);
      ring_elem a = P->make_flat_term(one, mon);
      mat.append(P->make_vec(0,a));
    }
  
  // Remove the garbage memory
  deletearray(exp);
  M->remove(mon);
  exponent_table_free(&E);

  // Finally, we sort them
  Matrix *result = mat.to_matrix();
  M2_arrayint perm = result->sort(0,-1);
  return result->sub_matrix(perm);
}

static void get_part_of_expvector(M2_arrayint vars, 
				  exponent big, 
				  int comp, 
				  exponent result)
// sets result[0..vars->len-1] with the corresponding exponents in 'big'
// sets result[vars->len] to be the component
// zeros out any variables in big which are placed into result.
//
// private routine for 'coeffs'.
{
  for (int j=0; j<vars->len; j++)
    {
      int v = vars->array[j];
      result[j] = big[v];
      big[v] = 0;
    }
  result[vars->len] = comp;
}

static vec coeffs_of_vec(exponent_table *E, M2_arrayint vars,
			 const FreeModule *F, vec f)
    // private routine for 'coeffs'.
#warning "coeffs_of_vec should maybe be in PolynomialRing"
{
  if (f == NULL) return 0;
  const PolynomialRing *P = F->get_ring()->cast_to_PolynomialRing();
  if (P == 0)
    return 0;
  const Monoid *M = P->getMonoid();
  int *mon = M->make_one();

  // At this point, we know that we have a polynomial ring
  int nvars = M->n_vars();
  int *exp = newarray(int,nvars);
  int *scratch_exp = newarray(int,1+vars->len);

  vec result = 0;
  for (vec g = f ; g != 0; g = g->next)
    {
      for (Nterm *h = g->coeff; h != 0; h = h->next)
	{
	  M->to_expvector(h->monom, exp);
	  get_part_of_expvector(vars, exp, g->comp, scratch_exp);
	  int val = exponent_table_get(E, scratch_exp);
	  if (val > 0)
	    {
	      M->from_expvector(exp, mon);
	      ring_elem t = P->make_flat_term(h->coeff, mon);
	      vec v = P->make_vec(val-1, t);
	      v->next = result;
	      result = v;
	    }
	}
    }
  deletearray(exp);
  deletearray(scratch_exp);
  M->remove(mon);
  P->vec_sort(result);
  return result;
}

MatrixOrNull *Matrix::coeffs(M2_arrayint vars, const Matrix *monoms) const
{
  // Given an array of variable indices, 'vars', and given
  // that 'monoms' and 'this' both have one row, makes a matrix
  // having number of rows = ncols(monoms), 
  //        number of cols = ncols(this),
  // whose (r,c) entry is the coefficient (in the other variables)
  // of this[0,c] in the monomial monoms[0,r].


  // Step 0: Do some error checking
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  if (P == 0)
    {
      ERROR("expected polynomial ring");
      return 0;
    }
  int nvars = P->n_vars();
  int nelements = monoms->n_cols();
  if (monoms->n_rows() != n_rows())
    {
      ERROR("expected matrices with the same number of rows");
      return 0;
    }
  for (unsigned int i=0; i<vars->len; i++)
    if (vars->array[i] < 0 || vars->array[i] >= nvars)
      {
	ERROR("coeffs: expected a set of variable indices");
	return 0;
      }

  // Step 1: Make an exponent_table of all of the monoms.
  // We set the value of the i th monomial to be 'i+1', since 0
  // indicates a non-existent entry.

  // The extra size in monomial refers to the component:
  exponent_table *E = exponent_table_new(nelements, 1 + vars->len); 
  exponent EXP = newarray(int,nvars);
  for (int i=0; i<nelements; i++)
    {
      vec v = monoms->elem(i);
      if (v == 0)
	{
	  ERROR("expected non-zero column");
	  return 0;
	}
      ring_elem f = v->coeff;
      const int *m = P->lead_flat_monomial(f);
      P->getMonoid()->to_expvector(m, EXP);

      // grab only that part of the monomial we need
      exponent e = newarray(int, 1 + vars->len);
      get_part_of_expvector(vars, EXP, v->comp, e);
      exponent_table_put(E, e, i+1);
    }

  // Step 2: for each vector column of 'this'
  //     create a column, and put this vector into result.

  MatrixConstructor mat(P->make_FreeModule(nelements), 0);
  for (int i=0; i<n_cols(); i++)
    mat.append(coeffs_of_vec(E, vars, rows(), elem(i)));

  return mat.to_matrix();
}

MonomialIdeal *Matrix::make_monideal(int n) const
{
  const PolyRing *P = get_ring()->cast_to_PolyRing();
  if (P == 0)
    {
      ERROR("expected polynomial ring with no fractions");
      return 0;
    }
#warning "make this work with fraction rings too"
  const Monoid *M = P->getMonoid();
  queue <Bag *> new_elems;
  for (int i=0; i<n_cols(); i++)
    {
      vec v = elem(i);
      if (v == 0) continue;
      const vecterm * w = P->vec_find_lead_term(rows(), v);
      if (w->comp != n) continue;
      Bag *b = new Bag(i);
      M->to_varpower(P->lead_flat_monomial(w->coeff), b->monom());
      new_elems.insert(b);      
    }

  // If the base ring is a quotient ring, include these lead monomials.
  if (P->is_quotient_ring())
    {
      const MonomialIdeal *Rideal = P->get_quotient_monomials();
      for (Index<MonomialIdeal> j = Rideal->first(); j.valid(); j++)
	{
	  Bag *b = new Bag(-1, (*Rideal)[j]->monom());
	  new_elems.insert(b);
	}
    }

  MonomialIdeal *result = new MonomialIdeal(P, new_elems);
  return result;
}

MonomialIdeal *Matrix::make_skew_monideal(int n) const
{
  MonomialIdeal *result = make_monideal(n);
  if (result == 0) return 0;
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  assert(P != 0);
  if (P->is_skew_commutative())
    {
      const Monoid *M = P->Nmonoms();
      intarray vp;
      for (int i=0; i<M->n_vars(); i++)
	if (P->is_skew_var(i))
	  {
	    vp.shrink(0);
	    varpower::var(i,2,vp);
	    Bag *b = new Bag(-1, vp);
	    result->insert_minimal(b);
	  }
    }
  return result;
}

MonomialIdeal *Matrix::make_basis_monideal(int n) const
{
#warning "make_basis_monideal doesn't handle fractions"
#warning "include the squares of the skew communting vars?"
  const PolyRing *P = get_ring()->cast_to_PolyRing();
  if (P == 0)
    {
      ERROR("expected polynomial ring");
      return 0;
    }
  const Monoid *M = P->getMonoid();
  intarray vp;
  queue <Bag *> new_elems;
  bool coeffsZZ = (P->coefficient_type() == Ring::COEFF_ZZ);
  int nlogical = P->getLogicalMonoid()->n_vars();
  int nothers = P->n_vars() - nlogical;
  exponents exp = newarray(int,P->n_vars());
  for (int i=0; i<n_cols(); i++)
    {
      vec v = elem(i);
      if (v == 0) continue;
      const vecterm * w = P->vec_find_lead_term(rows(), v);
      if (w->comp != n) continue;
      if (coeffsZZ && !globalZZ->is_unit(P->lead_flat_coeff(w->coeff)))
	continue;
      M->to_expvector(P->lead_flat_monomial(w->coeff), exp);
      if (!ntuple::is_one(nothers, exp + nlogical))
	continue;
      vp.shrink(0);
      varpower::from_ntuple(P->getLogicalMonoid()->n_vars(), exp, vp);
      Bag *b = new Bag(i, vp);
      new_elems.insert(b);      
    }

  // If the base ring is a quotient ring, include these lead monomials.
  for (int i=0; i<P->n_quotients(); i++)
    {
      Nterm *f = P->quotient_element(i);
      if (coeffsZZ && !globalZZ->is_unit(f->coeff))
	continue;
      M->to_expvector(f->monom, exp); // flat monomial
      if (!ntuple::is_one(nothers, exp + nlogical))
	continue;
      vp.shrink(0);
      varpower::from_ntuple(P->getLogicalMonoid()->n_vars(), exp, vp);
      Bag *b = new Bag(-1, vp);
      new_elems.insert(b);
    }

  MonomialIdeal *result = new MonomialIdeal(P, new_elems);
  deletearray(exp);
  return result;
}

int Matrix::dimension() const
{
  const PolynomialRing *P = get_ring()->cast_to_PolynomialRing();
  const Ring *K = (P != 0 ? P->Ncoeffs() : get_ring());
  bool is_ZZ = K->is_ZZ();
  int base = (is_ZZ ? 1 : 0);
  int result = -1;
  if (P != 0)
    {
      int n = P->n_vars();
      for (int i=0; i<n_rows(); i++)
	{
	  MonomialIdeal *mi = make_skew_monideal(i);
	  AssociatedPrimes ap(mi);
	  int d = n - ap.codimension();
	  if (d > result) result = d;
	}
      if (result != -1) result += base;
      return result;
    }
  else
    {
      // This handles the case when the coefficients are a field, or ZZ
      int i,j;
      int *dims = newarray(int,n_rows());
      for (i=0; i<n_rows(); i++)
	dims[i] = base;
      for (j=0; j<n_cols(); j++)
	{
	  vec f = elem(j);
	  if (f == 0) continue;
	  if (dims[f->comp] == -1) continue;
	  if (K->is_unit(f->coeff))
	    dims[f->comp] = -1;
	  else
	    dims[f->comp] = 0;
	}
      for (i=0; i<n_rows(); i++)
	if (dims[i] > result) result = dims[i];
      deletearray(dims);
      return result;
    }
}




// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
