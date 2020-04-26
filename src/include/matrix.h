/* Copyright (C) Jose Adrian Rodriguez Fonollosa 2004.
 *               Universitat Politecnica de Catalunya, Barcelona, Spain.
 *
 * Permission to copy, use, modify, sell and distribute this software
 * is granted provided this copyright notice appears in all copies.
 * This software is provided "as is" without express or implied
 * warranty, and with no claim as to its suitability for any purpose.
 */
#ifndef _MATRIX_H
#define _MATRIX_H

#include <iostream>
#include <valarray>
#include <iostream>
#include <inttypes.h>
namespace upc {

template<class _Ty>
	class array : public std::valarray<_Ty> {
private:
	typedef std::valarray<_Ty> Tvalarray;
	typedef std::allocator<_Ty> _A;
public:
	typedef array<_Ty> _Myt;
	//	typedef typename _A::size_type size_type;
	typedef uint32_t size_type;
	typedef typename _A::pointer _Tptr;
	typedef typename _A::const_pointer _Ctptr;
	typedef typename _A::reference reference;
	typedef typename _A::const_reference const_reference;
	explicit array(unsigned int size = 0) : Tvalarray(size) {}
	_Ctptr v() const {
		_Myt& fv = (_Myt&) *this;
		return fv.v();
	}
	_Tptr v() {
		return &(*this)[0];
	}
	void reset() {
	  for (size_type i=0; i <Tvalarray::size() ; i++)
	    (*this)[i] = 0;
	}
 };

template<class _Ty>
inline std::ostream& operator<<(std::ostream& os, const array<_Ty>& f)
{
	typename array<_Ty>::size_type size = f.size();
	os.write((const char*) &size, sizeof(typename array<_Ty>::size_type));
	os.write((const char*) f.v(), size*sizeof(_Ty));
	return os;
}

template<class _Ty>
inline std::istream& operator>>(std::istream& is, array<_Ty>& f)
{
	typename array<_Ty>::size_type size = f.size();
	if (is.read((char*) &size, sizeof(typename array<_Ty>::size_type))) {
		f.resize(size);
		is.read((char*) f.v(), size*sizeof(_Ty));
	}
	return is;
}

template<class _Ty>
	class matrix {
private:
	typedef matrix<_Ty> _Myt;
	typedef array<_Ty> Tvector;
	typedef typename Tvector::_Ctptr _Ctptr;
	typedef typename Tvector::_Tptr _Tptr;
	typedef array<_Tptr> Pvector;
public:
	typedef typename Tvector::size_type size_type;
	explicit matrix(size_type nrow=0, size_type ncol=0) : m_v(nrow*ncol), m_p(nrow), m_nrow(nrow), m_ncol(ncol) {
		if (nrow>0 && ncol>0) {
			size_type i;
			m_p[0] = m_v.v();
			for (i=1; i<nrow; i++)
				m_p[i] = m_p[i-1] + ncol;
		}
	}

	 matrix(const _Myt &o):  m_v(o.m_v), m_p(o.m_nrow), m_nrow(o.m_nrow), m_ncol(o.m_ncol){
		if (m_nrow>0 && m_ncol>0) {
			size_type i;
			m_p[0] = m_v.v();
			for (i=1; i<m_nrow; i++)
				m_p[i] = m_p[i-1] + m_ncol;
		}
	}


	size_type nrow() const {return m_nrow; }
	size_type ncol() const {return m_ncol; }
	_Ctptr operator[](int i) const {return m_p[i]; }
	_Tptr operator[](int i) {return m_p[i]; }
	const _Ctptr* m() const {return m_p.v(); }
	_Tptr* m() {return m_p.v(); }
	void reset() {m_v.reset(); }
	void resize(size_type nrow, size_type ncol) {
		Tvector v(nrow*ncol);
		size_type i, j;
		for (i=0; i<nrow; i++) {
			for (j=0; j<ncol; j++) {
				if ((i<m_nrow) && (j<m_ncol))
					v[i*ncol+j] = (*this)[i][j];
				else
					v[i*ncol+j] = 0;
			}
		}
		m_nrow = nrow;
		m_ncol = ncol;
		m_v.resize(nrow*ncol);
		m_v = v;
		m_p.resize(nrow);
		if (nrow>0 && ncol>0) {
			size_type i;
			m_p[0] = m_v.v();
			for (i=1; i<nrow; i++)
				m_p[i] = m_p[i-1] + ncol;
		}
	}

	_Myt & operator=(const _Myt & other) {
	  m_nrow = other.m_nrow;
	  m_ncol = other.m_ncol;
	  m_v = other.m_v;
	  m_p.resize(m_nrow);
	  if (m_nrow>0 && m_ncol>0) {
	    m_p[0] = m_v.v();
	    for (size_type i=1; i<m_nrow; i++)
	      m_p[i] = m_p[i-1] + m_ncol;
	  }
	  return (*this);
	}

private:
	Tvector m_v;
	Pvector m_p;
	size_type m_nrow, m_ncol;
};

template<class _Ty>
inline std::ostream& operator<<(std::ostream& os, const matrix<_Ty>& f)
{
	typename matrix<_Ty>::size_type nrow = f.nrow();
	typename matrix<_Ty>::size_type ncol = f.ncol();
	os.write((const char*) &nrow, sizeof(typename matrix<_Ty>::size_type));
	os.write((const char*) &ncol, sizeof(typename matrix<_Ty>::size_type));
	os.write((const char*) f[0], nrow*ncol*sizeof(_Ty));
	return os;
}

template<class _Ty>
inline std::istream& operator>>(std::istream& is, matrix<_Ty>& f)
{
	typename matrix<_Ty>::size_type nrow, ncol;
	is.read((char*) &nrow, sizeof(typename matrix<_Ty>::size_type));
	is.read((char*) &ncol, sizeof(typename matrix<_Ty>::size_type));
	f.resize(nrow, ncol);
	is.read((char*) f[0], nrow*ncol*sizeof(_Ty));
	return is;
}

typedef array<int>     ivector;
typedef matrix<int>    imatrix;
typedef array<float>   fvector;
typedef matrix<float>  fmatrix;
typedef array<double>  dvector;
typedef matrix<double> dmatrix;

}

#endif
