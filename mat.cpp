// Copyright 2005  Michael E. Stillman

#include "dmat.hpp"
#include "smat.hpp"
#include "mat.hpp"

#include "coeffrings.hpp"
#include "matrixcon.hpp"
#include "matrix.hpp"

#include "lapack.hpp"

MutableMatrixXXX *MutableMatrixXXX::zero_matrix(const Ring *R, 
						long nrows, 
						long ncols, 
						bool dense)
{
  if (nrows < 0 | ncols < 0)
    {
      ERROR("expected non-negative number of rows or columns");
      return 0;
    }
  const Z_mod *KZZp = R->cast_to_Z_mod();
  if (KZZp != 0)
    {
      if (dense)
	{
	  return MutableMat<CoefficientRingZZp, DMat<CoefficientRingZZp> >
	    ::zero_matrix(KZZp,nrows,ncols);
	}
      else
	return MutableMat<CoefficientRingZZp, SMat<CoefficientRingZZp> >
	    ::zero_matrix(KZZp,nrows,ncols);
    }
  if (R == globalZZ)
    {
 #warning "change to NTL mat_ZZ"
      if (dense)
	{
	  return MutableMat<CoefficientRingZZ_NTL, DMat<CoefficientRingZZ_NTL> >
	    ::zero_matrix(globalZZ,nrows,ncols);
	}
      else
	  return MutableMat<CoefficientRingZZ_NTL, SMat<CoefficientRingZZ_NTL> >
	    ::zero_matrix(globalZZ,nrows,ncols);
    }
  if (R == globalRR)
    {
      if (dense)
	{
	  return MutableMat<CoefficientRingRR, DMat<CoefficientRingRR> >
	    ::zero_matrix(globalRR,nrows,ncols);
	}
      else
	return MutableMat<CoefficientRingRR, SMat<CoefficientRingRR> >
	    ::zero_matrix(globalRR,nrows,ncols);
    }
  if (R == globalCC)
    {
      if (dense)
	{
	  return MutableMat<CoefficientRingCC, DMat<CoefficientRingCC> >
	    ::zero_matrix(globalCC,nrows,ncols);
	}
      else
	return MutableMat<CoefficientRingCC, SMat<CoefficientRingCC> >
	    ::zero_matrix(globalCC,nrows,ncols);
    }
  // In this case, we just use ring elem arithmetic
  if (dense)
    return MutableMat<CoefficientRingR, DMat<CoefficientRingR> >
            ::zero_matrix(R,nrows,ncols);
  else
    return MutableMat<CoefficientRingR, SMat<CoefficientRingR> >
            ::zero_matrix(R,nrows,ncols);
  const GF *KGF = R->cast_to_GF();
  if (KGF != 0)
    {
    }
  ERROR("mutable matrices over this ring are not yet implemented");
  return 0;
}

MutableMatrixXXX *MutableMatrixXXX::identity(const Ring *R, int nrows, bool dense)
{
  MutableMatrixXXX *result = MutableMatrixXXX::zero_matrix(R,nrows,nrows,dense);
  ring_elem one = R->one();
  for (int i=0; i<nrows; i++)
    result->set_entry(i,i,one);
  return result;
}

MutableMatrixXXX *MutableMatrixXXX::from_matrix(const Matrix *m, bool prefer_dense)
{
  MutableMatrixXXX *result = zero_matrix(m->get_ring(), 
					 m->n_rows(), 
					 m->n_cols(), 
					 prefer_dense);
  Matrix::iterator i(m);
  for (int c=0; c<m->n_cols(); c++)
    {
      for (i.set(c); i.valid(); i.next())
	result->set_entry(i.row(), c, i.entry());
    }
  return result;
}

Matrix *MutableMatrixXXX::to_matrix() const
{
  int nrows = n_rows();
  int ncols = n_cols();
  FreeModule *F = get_ring()->make_FreeModule(nrows);
  MatrixConstructor result(F,ncols);
  ring_elem f;
  iterator *i = begin();
  for (int c=0; c<ncols; c++)
    {
      ring_elem a;
      for (i->set(c); i->valid(); i->next())
	{
	  i->copy_ring_elem(a);
	  result.set_entry(i->row(), c, a);
	}
    }
  delete i;
  result.compute_column_degrees();
  return result.to_matrix();
}

void MutableMatrixXXX::text_out(buffer &o) const
{
  const Ring *R = get_ring();
  int nrows = n_rows();
  int ncols = n_cols();
  buffer *p = newarray(buffer,nrows);
  int r;
  for (int c=0; c<ncols; c++)
    {
      int maxcount = 0;
      for (r=0; r<nrows; r++)
	{
	  ring_elem f;
	  get_entry(r,c,f);
	  if (!R->is_zero(f))
	    R->elem_text_out(p[r], f);
	  else 
	    p[r] << ".";
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

bool MutableMatrixXXX::set_values(M2_arrayint rows,
				  M2_arrayint cols,
				  RingElement_array *values)
{
  if (rows->len != cols->len || rows->len != values->len)
    return false;
  for (long i=rows->len-1; i>=0; i--)
    {
      if (!set_entry(rows->array[i], cols->array[i], values->array[i]->get_value()))
	return false;
    }
  return true;
}


template<typename CoeffRing, typename MatType>
Mat_ZZp *MutableMat<CoeffRing,MatType>::get_mat_ZZp()
{
  return 0;
}

Mat_ZZp *MutableMat<CoefficientRingZZp,Mat<CoefficientRingZZp> >
  ::get_mat_ZZp()
{
  return &mat;
}

template<typename CoeffRing, typename MatType>
Mat_RR *MutableMat<CoeffRing,MatType>::get_mat_RR()
{
  return 0;
}

Mat_RR *MutableMat<CoefficientRingRR,Mat<CoefficientRingRR> >
  ::get_mat_RR()
{
  return &mat;
}


template<typename CoeffRing, typename MatType>
DMatRR * MutableMat<CoeffRing,MatType>::get_DMatRR()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
DMatCC * MutableMat<CoeffRing,MatType>::get_DMatCC()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
DMatZZp * MutableMat<CoeffRing,MatType>::get_DMatZZp()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
DMatZZ * MutableMat<CoeffRing,MatType>::get_DMatZZ()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
DMatR * MutableMat<CoeffRing,MatType>::get_DMatR()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
SMatRR * MutableMat<CoeffRing,MatType>::get_SMatRR()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
SMatCC * MutableMat<CoeffRing,MatType>::get_SMatCC()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
SMatZZp * MutableMat<CoeffRing,MatType>::get_SMatZZp()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
SMatZZ * MutableMat<CoeffRing,MatType>::get_SMatZZ()
{
  return 0;
}
template<typename CoeffRing, typename MatType>
SMatR * MutableMat<CoeffRing,MatType>::get_SMatR()
{
  return 0;
}


template<typename CoeffRing, typename MatType>
const DMatRR * MutableMat<CoeffRing,MatType>::get_DMatRR() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const DMatCC * MutableMat<CoeffRing,MatType>::get_DMatCC() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const DMatZZp * MutableMat<CoeffRing,MatType>::get_DMatZZp() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const DMatZZ * MutableMat<CoeffRing,MatType>::get_DMatZZ() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const DMatR * MutableMat<CoeffRing,MatType>::get_DMatR() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const SMatRR * MutableMat<CoeffRing,MatType>::get_SMatRR() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const SMatCC * MutableMat<CoeffRing,MatType>::get_SMatCC() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const SMatZZp * MutableMat<CoeffRing,MatType>::get_SMatZZp() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const SMatZZ * MutableMat<CoeffRing,MatType>::get_SMatZZ() const
{
  return 0;
}
template<typename CoeffRing, typename MatType>
const SMatR * MutableMat<CoeffRing,MatType>::get_SMatR() const
{
  return 0;
}

// Specializations of these
DMatRR * MutableMat<CoefficientRingRR,DMatRR>::get_DMatRR()
{
  return &mat;
}
DMatCC * MutableMat<CoefficientRingCC,DMatCC>::get_DMatCC()
{
  return &mat;
}
DMatZZp * MutableMat<CoefficientRingZZp,DMatZZp>::get_DMatZZp()
{
  return &mat;
}
DMatZZ * MutableMat<CoefficientRingZZ_NTL,DMatZZ>::get_DMatZZ()
{
  return &mat;
}
DMatR * MutableMat<CoefficientRingR,DMatR>::get_DMatR()
{
  return &mat;
}

SMatRR * MutableMat<CoefficientRingRR,SMatRR>::get_SMatRR()
{
  return &mat;
}
SMatCC * MutableMat<CoefficientRingCC,SMatCC>::get_SMatCC()
{
  return &mat;
}
SMatZZp * MutableMat<CoefficientRingZZp,SMatZZp>::get_SMatZZp()
{
  return &mat;
}
SMatZZ * MutableMat<CoefficientRingZZ_NTL,SMatZZ>::get_SMatZZ()
{
  return &mat;
}
SMatR * MutableMat<CoefficientRingR,SMatR>::get_SMatR()
{
  return &mat;
}

const DMatRR * MutableMat<CoefficientRingRR,DMatRR>::get_DMatRR() const
{
  return &mat;
}
const DMatCC * MutableMat<CoefficientRingCC,DMatCC>::get_DMatCC() const
{
  return &mat;
}
const DMatZZp * MutableMat<CoefficientRingZZp,DMatZZp>::get_DMatZZp() const
{
  return &mat;
}
const DMatZZ * MutableMat<CoefficientRingZZ_NTL,DMatZZ>::get_DMatZZ() const
{
  return &mat;
}
const DMatR * MutableMat<CoefficientRingR,DMatR>::get_DMatR() const
{
  return &mat;
}

const SMatRR * MutableMat<CoefficientRingRR,SMatRR>::get_SMatRR() const
{
  return &mat;
}
const SMatCC * MutableMat<CoefficientRingCC,SMatCC>::get_SMatCC() const
{
  return &mat;
}
const SMatZZp * MutableMat<CoefficientRingZZp,SMatZZp>::get_SMatZZp() const
{
  return &mat;
}
const SMatZZ * MutableMat<CoefficientRingZZ_NTL,SMatZZ>::get_SMatZZ() const
{
  return &mat;
}
const SMatR * MutableMat<CoefficientRingR,SMatR>::get_SMatR() const
{
  return &mat;
}

///////////////////////////////////
//// Linear algebra routines //////
///////////////////////////////////

template<typename CoeffRing, typename Mat>
bool MutableMat<CoeffRing,Mat>::solve(const MutableMatrixXXX *b, MutableMatrixXXX *x) const
  // resets x, find a basis of solutions for Ax=b
{
  ERROR("solving linear equations is not implemented for this ring and matrix type");
  return false;
}

template<typename CoeffRing, typename Mat>
void MutableMat<CoeffRing,Mat>::LU(MutableMatrixXXX *L, std::vector<int, gc_allocator<int> > &perm)
{
}

void MutableMat<CoefficientRingZZp, Mat<CoefficientRingZZp> >::LU(MutableMatrixXXX *L, std::vector<int, gc_allocator<int> > &perm)
{
}

template<typename CoeffRing, typename Mat>
bool MutableMat<CoeffRing,Mat>::eigenvalues(MutableMatrixXXX *eigenvals, bool is_symm_or_hermitian) const
{
  ERROR("eigenvalues requires dense mutable matrices over RR or CC");
  return false;
}

template<typename CoeffRing, typename Mat>
bool MutableMat<CoeffRing,Mat>::SVD(MutableMatrixXXX *Sigma,
				    MutableMatrixXXX *U,
				    MutableMatrixXXX *Vt,
				    bool use_divide_and_conquer) const
{
  ERROR("SVD requires dense mutable matrices over RR or CC");
  return false;
}

template<typename CoeffRing, typename Mat>
bool MutableMat<CoeffRing,Mat>::eigenvectors(MutableMatrixXXX *eigenvals,
					     MutableMatrixXXX *eigenvecs,
					     bool is_symm_or_hermitian) const
{
  ERROR("eigenvectors requires dense mutable matrices over RR or CC");
  return false;
}

template<typename CoeffRing, typename Mat>
bool MutableMat<CoeffRing,Mat>::least_squares(const MutableMatrixXXX *b,
					      MutableMatrixXXX *x,
					      bool assume_full_rank) const
{
  ERROR("least squares requires dense mutable matrices over RR or CC");
  return false;
}

bool MutableMat<CoefficientRingRR,DMatRR>::solve(const MutableMatrixXXX *b, MutableMatrixXXX *x) const
  // resets x, find a basis of solutions for Ax=b
{
  const DMatRR *A2 = get_DMatRR();
  const DMatRR *b2 = b->get_DMatRR();
  DMatRR *x2 = x->get_DMatRR();
  if (A2 == 0 || b2 == 0 || x2 == 0)
    {
      ERROR("requires dense mutable matrices over RR");
      return false;
    }
  return Lapack::solve(A2,b2,x2);
}

bool MutableMat<CoefficientRingCC,DMatCC>::solve(const MutableMatrixXXX *b, MutableMatrixXXX *x) const
  // resets x, find a basis of solutions for Ax=b
{
  const DMatCC *A2 = get_DMatCC();
  const DMatCC *b2 = b->get_DMatCC();
  DMatCC *x2 = x->get_DMatCC();
  if (A2 == 0 || b2 == 0 || x2 == 0)
    {
      ERROR("requires dense mutable matrices over CC");
      return false;
    }
  return Lapack::solve(A2,b2,x2);
}

bool MutableMat<CoefficientRingRR,DMat<CoefficientRingRR> >::eigenvalues(MutableMatrixXXX *eigenvals, bool is_symm_or_hermitian) const
{
  // First check that the matrix 'eigenvals' is correct type
  if (is_symm_or_hermitian)
    {
      DMatRR * eig = eigenvals->get_DMatRR();
      if (eig == 0)
	{
	  ERROR("requires a dense mutable matrix over RR");
	  return false;
	}
      return Lapack::eigenvalues_symmetric(get_DMatRR(), eig);
    }
  else
    {
      DMat<CoefficientRingCC> * eig = eigenvals->get_DMatCC();
      if (eig == 0)
	{
	  ERROR("requires a dense mutable matrix over CC");
	  return false;
	}
      return Lapack::eigenvalues(get_DMatRR(), eig);
    }
}

bool MutableMat<CoefficientRingCC,DMat<CoefficientRingCC> >::eigenvalues(MutableMatrixXXX *eigenvals, bool is_symm_or_hermitian) const
{
  // First check that the matrix 'eigenvals' is correct type
  if (is_symm_or_hermitian)
    {
      DMatRR * eig = eigenvals->get_DMatRR();
      if (eig == 0)
	{
	  ERROR("requires a dense mutable matrix over RR");
	  return false;
	}
      return Lapack::eigenvalues_hermitian(get_DMatCC(), eig);
    }
  else
    {
      DMatCC * eig = eigenvals->get_DMatCC();
      if (eig == 0)
	{
	  ERROR("requires a dense mutable matrix over CC");
	  return false;
	}
      return Lapack::eigenvalues(get_DMatCC(), eig);
    }
}

bool MutableMat<CoefficientRingRR,DMatRR>::eigenvectors(MutableMatrixXXX *eigenvals,
							MutableMatrixXXX *eigenvecs,
							bool is_symm_or_hermitian) const
{
  const DMatRR *A2 = get_DMatRR();
  if (is_symm_or_hermitian)
    {
      DMatRR *eigvals2 = eigenvals->get_DMatRR();
      DMatRR *eigvecs2 = eigenvecs->get_DMatRR();
      if (eigvals2 == 0 || eigvecs2 == 0)
	{
	  ERROR("requires dense mutable matrices over RR");
	  return false;
	}
      return Lapack::eigenvectors_symmetric(A2,eigvals2,eigvecs2);
    }
  else
    {
      DMatCC *eigvals2 = eigenvals->get_DMatCC();
      DMatCC *eigvecs2 = eigenvecs->get_DMatCC();
      if (eigvals2 == 0 || eigvecs2 == 0)
	{
	  ERROR("requires dense mutable matrices over CC");
	  return false;
	}
      return Lapack::eigenvectors(A2,eigvals2,eigvecs2);
    }
}

bool MutableMat<CoefficientRingCC,DMatCC>::eigenvectors(MutableMatrixXXX *eigenvals,
							MutableMatrixXXX *eigenvecs,
							bool is_symm_or_hermitian) const
{
  const DMatCC *A2 = get_DMatCC();
  if (is_symm_or_hermitian)
    {
      DMatRR *eigvals2 = eigenvals->get_DMatRR();
      DMatCC *eigvecs2 = eigenvecs->get_DMatCC();
      if (eigvals2 == 0)
	{
	  ERROR("requires dense mutable matrix over RR");
	  return false;
	}
      if (eigvecs2 == 0)
	{
	  ERROR("requires dense mutable matrix over CC");
	  return false;
	}
      return Lapack::eigenvectors_hermitian(A2,eigvals2,eigvecs2);
    }
  else
    {
      DMatCC *eigvals2 = eigenvals->get_DMatCC();
      DMatCC *eigvecs2 = eigenvecs->get_DMatCC();
      if (eigvals2 == 0 || eigvecs2 == 0)
	{
	  ERROR("requires dense mutable matrices over CC");
	  return false;
	}
      return Lapack::eigenvectors(A2,eigvals2,eigvecs2);
    }
}

bool MutableMat<CoefficientRingRR,DMatRR>::SVD(MutableMatrixXXX *Sigma,
				    MutableMatrixXXX *U,
				    MutableMatrixXXX *VT,
				    bool use_divide_and_conquer) const
{
  const DMatRR *A2 = get_DMatRR();
  DMatRR *Sigma2 = Sigma->get_DMatRR();
  DMatRR *U2 = U->get_DMatRR();
  DMatRR *VT2 = VT->get_DMatRR();
  if (A2 == 0 || Sigma2 == 0 || U2 == 0 || VT2 == 0)
    {
      ERROR("requires dense mutable matrices over RR");
      return false;
    }
  
  if (use_divide_and_conquer)
    {
      return Lapack::SVD_divide_conquer(A2,Sigma2,U2,VT2);
    }
  else
    {
      return Lapack::SVD(A2,Sigma2,U2,VT2);
    }
}

bool MutableMat<CoefficientRingCC,DMatCC>::SVD(MutableMatrixXXX *Sigma,
				    MutableMatrixXXX *U,
				    MutableMatrixXXX *VT,
				    bool use_divide_and_conquer) const
{
  const DMatCC *A2 = get_DMatCC();
  DMatRR *Sigma2 = Sigma->get_DMatRR();
  DMatCC *U2 = U->get_DMatCC();
  DMatCC *VT2 = VT->get_DMatCC();
  if (Sigma2 == 0)
    {
      ERROR("requires dense mutable matrix over RR");
      return false;
    }
  if (A2 == 0 || U2 == 0 || VT2 == 0)
    {
      ERROR("requires dense mutable matrices over CC");
      return false;
    }
  
  if (use_divide_and_conquer)
    {
      return Lapack::SVD_divide_conquer(A2,Sigma2,U2,VT2);
    }
  else
    {
      return Lapack::SVD(A2,Sigma2,U2,VT2);
    }
}

bool MutableMat<CoefficientRingRR,DMatRR>::least_squares(const MutableMatrixXXX *b,
							 MutableMatrixXXX *x,
							 bool assume_full_rank) const
{
  const DMatRR *A2 = get_DMatRR();
  const DMatRR *b2 = b->get_DMatRR();
  DMatRR *x2 = x->get_DMatRR();
  
  if (A2 == 0 || b2 == 0 || x2 == 0)
    {
      ERROR("requires dense mutable matrices over RR");
      return false;
    }
  
  if (assume_full_rank)
    {
      return Lapack::least_squares(A2,b2,x2);
    }
  else
    {
      return Lapack::least_squares_deficient(A2,b2,x2);
    }
}

bool MutableMat<CoefficientRingCC,DMatCC>::least_squares(const MutableMatrixXXX *b,
							 MutableMatrixXXX *x,
							 bool assume_full_rank) const
{
  const DMatCC *A2 = get_DMatCC();
  const DMatCC *b2 = b->get_DMatCC();
  DMatCC *x2 = x->get_DMatCC();
  
  if (A2 == 0 || b2 == 0 || x2 == 0)
    {
      ERROR("requires dense mutable matrices over CC");
      return false;
    }
  
  if (assume_full_rank)
    {
      return Lapack::least_squares(A2,b2,x2);
    }
  else
    {
      return Lapack::least_squares_deficient(A2,b2,x2);
    }
}

template class Mat<CoefficientRingZZp>;
template class MutableMat< CoefficientRingZZp, Mat<CoefficientRingZZp> >;

template class MutableMat< CoefficientRingZZp, DMat<CoefficientRingZZp> >;
template class MutableMat< CoefficientRingRR, DMat<CoefficientRingRR> >;
template class MutableMat< CoefficientRingCC, DMat<CoefficientRingCC> >;
template class MutableMat< CoefficientRingZZ_NTL, DMat<CoefficientRingZZ_NTL> >;
template class MutableMat< CoefficientRingR, DMat<CoefficientRingR> >;

template class MutableMat< CoefficientRingZZp, SMat<CoefficientRingZZp> >;
template class MutableMat< CoefficientRingRR, SMat<CoefficientRingRR> >;
template class MutableMat< CoefficientRingCC, SMat<CoefficientRingCC> >;
template class MutableMat< CoefficientRingZZ_NTL, SMat<CoefficientRingZZ_NTL> >;
template class MutableMat< CoefficientRingR, SMat<CoefficientRingR> >;


// Local Variables:
// compile-command: "make -C $M2BUILDDIR/Macaulay2/e "
// End:
