// Copyright 1995  Michael E. Stillman

#ifndef _mutablemat_hpp_
#define _mutablemat_hpp_

#include "matrix.hpp"

class SparseMutableMatrix;
class DenseMutableMatrix;
class DenseMutableMatrixRing;
class DenseMutableMatrixRR;
class DenseMutableMatrixCC;

class MutableMatrix : public mutable_object
{
protected:
  const Ring *R;

  int nrows;  // These can change, but in doing so, extra space will be allocated?
  int ncols;

  MutableMatrix *rowOps;
  MutableMatrix *colOps;	// Transpose of column matrix

  MutableMatrix() : R(0), nrows(0), ncols(0) {}
  MutableMatrix(const Ring *R0) : R(R0), nrows(0), ncols(0) {}

  bool error_column_bound(int c) const; // Sets an error if false is returned.

  bool error_row_bound(int r) const; // Sets an error if false is returned.

  virtual ~MutableMatrix() {}
public:
  const Ring * get_ring() const { return R; }
  int n_rows() const { return nrows; }
  int n_cols() const { return ncols; }

  bool setRowChangeMatrix(MutableMatrix *rops);
  // Returns false if the ring is wrong, 
  // or the number of columns of rops is not the
  // number of rows of 'this'.

  bool setColumnChangeMatrix(MutableMatrix *cops);
  // Returns false if the ring is wrong, 
  // or the number of columns of rops is not the
  // number of columns of 'this'.

  MutableMatrix *getRowChangeMatrix();

  MutableMatrix *getColumnChangeMatrix();

  static MutableMatrix *zero_matrix(const Ring *R, int nrows, int ncols, bool dense);
  // If the ring is RR or CC, and dense is true, then MutableMatrixRR or 
  // MutableMatrixCC will be used.

  static MutableMatrix *identity(const Ring *R, int nrows, bool dense);
  // If the ring is RR or CC, and dense is true, then MutableMatrixRR or 
  // MutableMatrixCC will be used.

  static MutableMatrix *from_matrix(const Matrix *N, bool is_dense);
  // If the ring is RR or CC, and dense is true, then MutableMatrixRR or 
  // MutableMatrixCC will be used.

  virtual Matrix *to_matrix() const = 0;

  virtual MutableMatrix *copy(bool prefer_dense) const = 0;

  virtual SparseMutableMatrix * cast_to_SparseMutableMatrix() { return 0; }
  virtual DenseMutableMatrix * cast_to_DenseMutableMatrix() { return 0; }
  virtual DenseMutableMatrixRing * cast_to_DenseMutableMatrixRing() { return 0; }
  virtual DenseMutableMatrixRR * cast_to_DenseMutableMatrixRR() { return 0; }
  virtual DenseMutableMatrixCC * cast_to_DenseMutableMatrixCC() { return 0; }

  virtual void text_out(buffer &o) const;
public:
  ///////////////////////////////
  // Row and column operations //
  ///////////////////////////////
  // The following routines return false if one of the row or columns given
  // is out of range.

  virtual bool get_entry(int r, int c, ring_elem &result) const = 0;
  // Returns false if (r,c) is out of range.

  virtual bool set_entry(int r, int c, const ring_elem a) = 0;
  // Returns false if (r,c) is out of range, or the ring of a is wrong.

  virtual bool interchange_rows(int i, int j, bool do_recording=true) = 0;
  /* swap rows: row(i) <--> row(j) */

  virtual bool interchange_columns(int i, int j, bool do_recording=true) = 0;
  /* swap columns: column(i) <--> column(j) */

  virtual bool scale_row(ring_elem r, int i, bool opposite_mult, bool do_recording=true) = 0;
  /* row(i) <- r * row(i) */

  virtual bool scale_column(ring_elem r, int i, bool opposite_mult, bool do_recording=true) = 0;
  /* column(i) <- r * column(i) */

  virtual bool row_op(int i, ring_elem r, int j, bool opposite_mult, bool do_recording=true) = 0;
  /* row(i) <- row(i) + r * row(j) */

  virtual bool column_op(int i, ring_elem r, int j, bool opposite_mult, bool do_recording=true) = 0;
  /* column(i) <- column(i) + r * column(j) */

  virtual bool dot_product(int i, int j, ring_elem &result) const = 0;
  /* dot product of columns i and j */

  ///////////////////////////////
  // Matrix operations //////////
  ///////////////////////////////

  virtual bool is_zero() const = 0;

  virtual bool is_equal(const MutableMatrix *B) const = 0;

  virtual bool set_values(M2_arrayint rows,
			  M2_arrayint cols,
			  RingElement_array *values) = 0;

  virtual MutableMatrixOrNull * add(const MutableMatrix *B) const = 0; 
  // return this + B.  return NULL of sizes or types do not match.
  // note: can add a sparse + dense
  //       can add a matrix over RR and one over CC and/or one over ZZ.

  virtual MutableMatrixOrNull * subtract(const MutableMatrix *B) const = 0; 
  // return this - B.  return NULL of sizes or types do not match.
  // note: can subtract a sparse + dense
  //       can subtract a matrix over RR and one over CC and/or one over ZZ.

  virtual MutableMatrixOrNull * mult(const MutableMatrix *B,
				     M2_bool opposite_mult) const = 0; 
  // return this * B.  return NULL of sizes or types do not match.
  // note: can mult a sparse + dense
  //       can mult a matrix over RR and one over CC and/or one over ZZ.

  virtual MutableMatrixOrNull * mult(const RingElement *f,
				     M2_bool opposite_mult) const = 0; 
  // return f*this.  return NULL of sizes or types do not match.

  virtual MutableMatrix * negate() const = 0;

  virtual MutableMatrix * submatrix(const M2_arrayint rows, const M2_arrayint cols) const = 0;

  virtual MutableMatrix * submatrix(const M2_arrayint cols) const = 0;
};

inline bool MutableMatrix::error_column_bound(int c) const
{
  if (c < 0 || c >= ncols)
    {
      ERROR("column out of range");
      return true;
    }
  return false;
}

inline bool MutableMatrix::error_row_bound(int r) const
{
  if (r < 0 || r >= nrows)
    {
      ERROR("row out of range");
      return true;
    }
  return false;
}

class SparseMutableMatrix : public MutableMatrix
{
public:
  static SparseMutableMatrix *zero_matrix(const Ring *R, int nrows, int ncols);

  virtual SparseMutableMatrix * cast_to_SparseMutableMatrix() { return this; }
};

class DenseMutableMatrix : public MutableMatrix
{
protected:
  DenseMutableMatrix() {}
  DenseMutableMatrix(const Ring *R0) : MutableMatrix(R0) {}
  virtual ~DenseMutableMatrix() {}
public:
  virtual DenseMutableMatrix * cast_to_DenseMutableMatrix() { return this; }
};

class DenseMutableMatrixRR : public DenseMutableMatrix
{
  double *array_; // array has length nrows*ncols
                  // columns stored one after another

  void initialize(int nrows, int ncols, double *array);
public:
  static DenseMutableMatrixRR *zero_matrix(int nrows, int ncols);

  virtual DenseMutableMatrixRR * cast_to_DenseMutableMatrixRR() { return this; }
};

class DenseMutableMatrixCC : public DenseMutableMatrix
{
  double *array_;  // stored as double to assure contiguous memory
                   // array has length 2*nrows*ncols
                   // columns stored one after another

  void initialize(int nrows, int ncols, double *array);
public:
  static DenseMutableMatrixCC *zero_matrix(int nrows, int ncols);

  virtual DenseMutableMatrixCC * cast_to_DenseMutableMatrixCC() { return this; }
};

#if 0

class MutableMatrix : public Matrix
{
  MutableMatrix *rowOps;
  MutableMatrix *colOps;	// Transpose of column matrix

  vector<int> colSize;
  vector<int> rowSize;

  MutableMatrix(const FreeModule *target, int ncols);
public:
  static SparseMutableMatrix *identity(const Ring *K, int n);

  MutableMatrix * cast_to_MutableMatrix() { return this; }

  void setRowChangeMatrix(MutableMatrix *rops);

  void setColumnChangeMatrix(MutableMatrix *cops);

  MutableMatrix *getRowChangeMatrix();

  MutableMatrix *getColumnChangeMatrix();




  bool set_entry(int r, int c, const ring_elem a);
  // Returns false if (r,c) is out of range, or the ring of a is wrong.

  bool interchange_rows(int i, int j, bool doRecording=true);
  /* swap rows: row(i) <--> row(j) */

  bool interchange_columns(int i, int j, bool doRecording=true);
  /* swap columns: column(i) <--> column(j) */

  bool scale_row(ring_elem r, int i, bool opposite_mult, bool doRecording=true);
  /* row(i) <- r * row(i) */

  bool scale_column(ring_elem r, int i, bool opposite_mult, bool doRecording=true);
  /* column(i) <- r * column(i) */

  bool row_op(int i, ring_elem r, int j, bool opposite_mult, bool doRecording=true);
  /* row(i) <- row(i) + r * row(j) */

  bool column_op(int i, ring_elem r, int j, bool opposite_mult, bool doRecording=true);
  /* column(i) <- column(i) + r * column(j) */






  int numNonZeroRow(int r) const;
  int numNonZeroColumn(int c) const;

  int leadRow(int c) const;  // -1 means this sparse_vector is zero.
  ring_elem leadCoefficient(int c) const; // Can be zero.

  bool getEntry(int r, int c, ring_elem &result) const;

  void column2by2(int c1, int c2, 
		  ring_elem a1, ring_elem a2,
		  ring_elem b1, ring_elem b2,
		  bool doRecording=true);

  void row2by2(int r1, int r2, 
	       ring_elem a1, ring_elem a2,
	       ring_elem b1, ring_elem b2,
	       bool doRecording=true);

  void divideRow(int r, ring_elem a, bool doRecording=true);
  void divideColumn(int c, ring_elem a, bool doRecording=true);

  ring_elem dotProduct(int c1, int c2) const;

  void columnReduce(int pivot_column, int c, bool doRecording=true);
  // Let c1, c2 denote these two columns pivot_column, c.
  // If a1,a2 are the lead coefficients of c1, c2, respectively, then set:
  //   matrix[c2] = c2 - (a2/a1) * c1.
  // If a1 is '1', then division is not performed.

  // Let c1, c2 denote these two columns pivot_column, c.
  // If a1,a2 are the lead coefficients of c1, c2, respectively,
  // and if x*a1+y*a2=d=gcd(a1,a2), then set
  //   matrix[column c1] = x*c1+y*c2
  //   matrix[column c2] = (a1/d) c1 - (a2/d) c2
  // This assumes that gcdExtended is defined in the base ring.
  void gcdColumnReduce(int pivot_column, int c, bool doRecording=true);
  void gcdColumnReduce(int r, int pivot_column, int c, bool doRecording=true);
  void gcdRowReduce(int c, int pivot_row, int r, bool doRecording=true);

  void sortColumns(int lo, int hi, bool doRecording=true);

  void permuteColumns(int lo, int hi, int *permutation, bool doRecording=true);
  // Permute the columns of the matrix, at least those in the range lo..hi.
  // i.e. matrix[c] is moved to matrix[sortval[c]].

  void normalizeColumn(int c, bool doRecording=true);

  // Find a good 'one' pivot location, if any '1's or '-1's
  void setSizes(int c_lo, int c_hi);
  bool findGoodUnitPivot(int c_lo, int c_hi, int &r, int &c, int &best);
  bool findGoodPivot(int c_lo, int c_hi, int &r, int &c, int &best);

  // Harry's routines
  void reducePivots();
  
}

#endif
#endif


// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
