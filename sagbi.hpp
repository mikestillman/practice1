// Copyright 1997  Michael E. Stillman

#ifndef _sagbi_hh_
#define _sagbi_hh_

#include "matrix.hpp"
#include "comp.hpp"
#include "gb_comp.hpp"

class sagbi
{
public:
  static vec subduct(const FreeModule *F,
		     vec f,
		     const RingMap &phi,
		     gb_comp *J);

  static Matrix subduct(const Matrix &m, 
			const RingMap &phi, 
			gb_comp *J);
};

class sagbi_comp : public gb_comp
{
  struct sagbi_elem {
    sagbi_elem *next;
    vec elem;
  };

  int _n_left;			// Number elements in pending lists.
  int _n_iterations;
  int _max_degree;
  array<Matrix> Pending;        // over R

  Matrix G;			// Sagbi basis as so far computed, over R.
  PolynomialRing *RS;
  binomialGB_comp *J;
  RingMap *Gmap;
  RingMap *RtoRS;
  RingMap *RStoR;

  void append_to_basis(Matrix &m);  // Adds to G, also modifies J,RS,...
  Matrix grab_lowest_degree();
  void row_reduce(Matrix &m);  // Modifies m.
  void insert_to_pending(Matrix &m);
public:
  // creation
  sagbi_comp(const Matrix &m);
  ~sagbi_comp();

  void enlarge(const Ring *R, int *wts);
  void add_generators(const Matrix &m);
  int calc(const int *deg, const intarray &stop_conditions);

  Matrix reduce(const Matrix &m, Matrix &lift);
  Vector reduce(const Vector &v, Vector &lift);

  virtual int contains(const Matrix &m);
  virtual bool is_equal(const gb_comp *q);
  
  // obtaining: mingens matrix, GB matrix, change of basis matrix, stats.
  Matrix min_gens_matrix();
  Matrix initial_matrix(int n);
  Matrix gb_matrix();
  Matrix change_matrix();
  Matrix syz_matrix();
  void stats() const;

public:
  // infrastructure
  friend void i_stashes();
  static stash *mystash;
  void *operator new(size_t) { return mystash->new_elem(); }
  void operator delete(void *p) { mystash->delete_elem(p); }

  sagbi_comp * cast_to_sagbi_comp() { return this; }
  const sagbi_comp * cast_to_sagbi_comp() const { return this; }

  void bin_out(ostream &) const {}
  const char * type_name         () const { return "sagbi_comp"; }
  void text_out(ostream &o) const { o << "sagbi_comp"; }

  int length_of() const { return 0; }
};
#endif
