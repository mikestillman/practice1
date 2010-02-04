#ifndef _m2interface_hh_
#define _m2interface_hh_

struct M2Expr
{
  unsigned short type_;
  void* ptr_;
};

inline bool M2ExprIsSequence(M2Expr* expr)
{
  return expr->type_ == 36;
}

struct M2Sequence
{
  int m_Length;
  M2Expr* m_Exprs[0];
};

inline int M2SequenceLength(M2Sequence* seq)
{
  return seq->m_Length;
}

inline M2Expr* M2SequenceGetExpr(M2Sequence* seq, int i)
{
  return seq->m_Exprs[i];
}
inline void M2SequenceSetExpr(M2Sequence* seq, M2Expr* expr, int i)
{
  seq->m_Exprs[i]=expr;
}
#endif
