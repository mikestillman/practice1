// Copyright 1995  Michael E. Stillman

#ifndef _matrix_hh_
#define _matrix_hh_

#include "monoid.hpp"
#include "freemod.hpp"
#include "monideal.hpp"

class Matrix : public mutable_object
{
  FreeModule *_rows;
  FreeModule *_cols;
  int *_degree_shift;		// An element of the degree monoid
  array<vec> _entries;

  friend class FreeModule;
private:
  void initialize(const FreeModule *r, const FreeModule *c, const int *deg);

  vec &elem(int i) { return _entries[i]; }
  const vec &elem(int i) const { return _entries[i]; }

  // These two routines are private to 'coeffs'
  vec strip_vector(vec &f, const int *vars, 
		       const FreeModule *F, vec &vmonom) const;
  int moneq(const int *exp, int *m, const int *vars, int *exp2) const;

  void k_basis0(int topvar) const;
  void k_basis1(int topvar) const;
  void k_basis_insert() const;
  
  void symm1(Matrix * &result, 
	     vec f,	       // product so far generated
	     int lastn,        // can use lastn..n_cols()-1 in product
	     int pow) const;   // remaining power to take

public:
  Matrix(const FreeModule *r, 
	 const FreeModule *c,
	 const int *deg);

  Matrix(const FreeModule *r, 
	 const FreeModule *c);

  Matrix(const FreeModule *r);

  Matrix(const MonomialIdeal * mi);

  static const MatrixOrNull * make(const FreeModule *target,
				   const Vector_array *V);

  static const MatrixOrNull * make(const FreeModule *target,
				   const FreeModule *source,
				   const M2_arrayint deg,
				   const Vector_array *V);

  static const MatrixOrNull * make(const FreeModule *target,
				   const FreeModule *source,
				   const M2_arrayint deg,
				   const Matrix *M);

  static const Matrix * make(const MonomialIdeal * mi);

  const Ring *get_ring() const { return rows()->get_ring(); }
  const Monoid *degree_monoid() const { return get_ring()->degree_monoid(); }

  vec &operator[](int i) { return _entries[i]; }
  const vec &operator[](int i) const { return _entries[i]; }
  ring_elem elem(int i, int j) const 
    { return rows()->get_coefficient(elem(j), i); }

  FreeModule *rows() { return _rows; }
  FreeModule *cols() { return _cols; }
  const FreeModule *rows() const { return _rows; }
  const FreeModule *cols() const { return _cols; }

  int n_rows() const { return rows()->rank(); }
  int n_cols() const { return cols()->rank(); }

  void append(vec v);
  void append(vec v, const int *d);
  void schreyer_append(vec v);

  // The degree shift
  const int *degree_shift() const { return _degree_shift; }
  //  int *degree_shift() { return _degree_shift; }

  // to/from monideals
  MonomialIdeal * make_monideal(int n) const;
  MonomialIdeal * make_skew_monideal(int n) const;
  void append_monideal(const MonomialIdeal *mi, int k);

  // Matrix operations
  MatrixOrNull *sub_matrix(const M2_arrayint r, const M2_arrayint c) const;
  MatrixOrNull *sub_matrix(const M2_arrayint c) const;
  Matrix *transpose() const;
  Matrix *operator+(const Matrix &m) const;
  Matrix *operator-() const;
  Matrix *operator-(const Matrix &m) const;
  Matrix *operator*(const ring_elem r) const;
  Matrix *operator*(const Matrix &r) const;
  Matrix *concat(const Matrix &m) const;

  static Matrix *identity(const FreeModule *F);
  static MatrixOrNull *zero(const FreeModule *F, const FreeModule *G);

  MatrixOrNull *koszul(int p) const;
  static MatrixOrNull *koszul(const Matrix *rows, const Matrix *cols);

  MatrixOrNull *reshape(const FreeModule *G, const FreeModule *H) const;
  static MatrixOrNull *flip(const FreeModule *G, const FreeModule *H);

  MatrixOrNull *direct_sum(const Matrix *m) const;
  MatrixOrNull *module_tensor(const Matrix *m) const;
  MatrixOrNull *tensor(const Matrix *m) const;
  MatrixOrNull *diff(const Matrix *m, int use_coef) const;
  MatrixOrNull *symm(int n) const;

  MatrixOrNull *coeffs(const int *vars, Matrix * &result_monoms) const;
  MatrixOrNull *coeffs(M2_arrayint vars, const M2_arrayint monoms) const;
  MatrixOrNull *monomials(M2_arrayint vars) const;

  Matrix *lead_var_coefficient(Matrix * &monoms) const;

  Matrix *k_basis(Matrix &bot, const int *d, int do_trunc) const;
  Matrix *k_basis(Matrix &bot) const;

  Matrix *exterior(int p,int strategy) const;
  Matrix *minors(int p,int strategy) const;
  Matrix *pfaffians(int p) const;
  static Matrix *wedge_product(int p, int q, const FreeModule *F);
//  static Matrix wedge_dual(int p, const FreeModule *F);

  // equality, zero
  bool is_equal(const Matrix &m) const;
  bool is_zero() const;

  // degrees
  int is_homogeneous() const;
  Matrix *homogenize(int v, const M2_arrayint wts) const;

  // Simplification of column set
  Matrix *simplify(int n) const;
  Matrix *auto_reduce() const;	// An error is given, if their are two lead terms
				// one which divides another.

  // Sorting the columns of the matrix (new positions into 'result')
  //  void sort(int degorder, int monorder, intarray &result) const;
  M2_arrayint sort(int degorder, int monorder) const;

  // Matrix selection
  Matrix *lead_term(int n=-1) const; // Select those monomials in each column
				  // which are maximal in the order under
				  // the first n weight vectors

  // Module operations
  int dimension() const;	// Compute the dimension of the quotient of the 
				// submodule generated by the lead terms of the
				// columns of the matrix, modulo any lead terms of the
				// presentation ideal of the ring of m.
				// Over ZZ, this gives the dimension over QQ.

private:
  void minimal_lead_terms_ZZ(intarray &result) const;
public:
  void minimal_lead_terms(intarray &result) const;

  M2_arrayint elim_vars(int nparts) const;
  M2_arrayint elim_keep(int nparts) const;
  Matrix *divide_by_var(int n, int maxd, int &maxdivided) const; // maxd<0 means divide by as much as possible

  static Matrix *random(const Ring *R, int r, int c);
  void text_out(buffer &o) const;
};

inline void Matrix::append(vec v, const int *d)
{
  _cols->append(d);
  _entries.append(v);
}

inline void Matrix::append(vec v)
{
  int *d = degree_monoid()->make_one();
  if (! rows()->is_zero(v)) rows()->degree(v, d);
  append(v,d);
  degree_monoid()->remove(d);
}

#endif

// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e"
// End:
