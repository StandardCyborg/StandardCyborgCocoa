#include "ramer_douglas_peucker.h"

#include "LinSpaced.h"
#include "find.h"
#include "list_to_matrix.h"
#include "cumsum.h"
#include "histc.h"
#include "project_to_line.h"
#include "placeholders.h"
#include "EPS.h"

template <typename DerivedP, typename DerivedS, typename DerivedJ>
IGL_INLINE void igl::ramer_douglas_peucker(
  const Eigen::MatrixBase<DerivedP> & P,
  const typename DerivedP::Scalar tol,
  Eigen::PlainObjectBase<DerivedS> & S,
  Eigen::PlainObjectBase<DerivedJ> & J)
{
  typedef typename DerivedP::Scalar Scalar;
  // number of vertices
  const int n = P.rows();
  // Trivial base case
  if(n <= 1)
  {
    J = DerivedJ::Zero(n);
    S = P;
    return;
  }
  Eigen::Array<bool,Eigen::Dynamic,1> I =
    Eigen::Array<bool,Eigen::Dynamic,1>::Constant(n,1,true);
  const auto stol = tol*tol;
  std::function<void(const int,const int)> simplify;
  simplify = [&I,&P,&stol,&simplify](const int ixs, const int ixe)->void
  {
    assert(ixe>ixs);
    Scalar sdmax = 0;
    typename Eigen::Matrix<Scalar,Eigen::Dynamic,1>::Index ixc = -1;
    if((ixe-ixs)>1)
    {
      Scalar sdes = (P.row(ixe)-P.row(ixs)).squaredNorm();
      Eigen::Matrix<Scalar,Eigen::Dynamic,1> sD;
      const auto & Pblock = P.block(ixs+1,0,((ixe+1)-ixs)-2,P.cols());
      if(sdes<=EPS<Scalar>())
      {
        sD = (Pblock.rowwise()-P.row(ixs)).rowwise().squaredNorm();
      }else
      {
        Eigen::Matrix<Scalar,Eigen::Dynamic,1> T;
        project_to_line(Pblock,P.row(ixs).eval(),P.row(ixe).eval(),T,sD);
      }
      sdmax = sD.maxCoeff(&ixc);
      // Index full P
      ixc = ixc+(ixs+1);
    }
    if(sdmax <= stol)
    {
      if(ixs != ixe-1)
      {
        I.block(ixs+1,0,((ixe+1)-ixs)-2,1).setConstant(false);
      }
    }else
    {
      simplify(ixs,ixc);
      simplify(ixc,ixe);
    }
  };
  simplify(0,n-1);
  igl::find(I,J);
  S = P(J.derived(),igl::placeholders::all);
}

template <
  typename DerivedP, 
  typename DerivedS, 
  typename DerivedJ,
  typename DerivedQ>
IGL_INLINE void igl::ramer_douglas_peucker(
  const Eigen::MatrixBase<DerivedP> & P,
  const typename DerivedP::Scalar tol,
  Eigen::PlainObjectBase<DerivedS> & S,
  Eigen::PlainObjectBase<DerivedJ> & J,
  Eigen::PlainObjectBase<DerivedQ> & Q)
{
  typedef typename DerivedP::Scalar Scalar;
  ramer_douglas_peucker(P,tol,S,J);
  const int n = P.rows();
  assert(n>=2 && "Curve should be at least 2 points");
  typedef Eigen::Matrix<Scalar,Eigen::Dynamic,1> VectorXS;
  // distance traveled along high-res curve
  VectorXS L(n);
  L(0) = 0;
  L.block(1,0,n-1,1) = (P.bottomRows(n-1)-P.topRows(n-1)).rowwise().norm();
  // Give extra on end
  VectorXS T;
  cumsum(L,1,T);
  T.conservativeResize(T.size()+1);
  T(T.size()-1) = T(T.size()-2);
  // index of coarse point before each fine vertex
  Eigen::VectorXi B;
  {
    Eigen::VectorXi N;
    histc(igl::LinSpaced<Eigen::VectorXi >(n,0,n-1),J,N,B);
  }
  // Add extra point at end
  J.conservativeResize(J.size()+1);
  J(J.size()-1) = J(J.size()-2);
  Eigen::VectorXi s,d;
  // Find index in original list of "start" vertices
  s = J(B);
  // Find index in original list of "destination" vertices
  d = J(B.array()+1);
  // Parameter between start and destination is linear in arc-length
  VectorXS Ts = T(s);
  VectorXS Td = T(d);
  T = ((T.head(T.size()-1)-Ts).array()/(Td-Ts).array()).eval();
  for(int t =0;t<T.size();t++)
  {
    if(!std::isfinite(T(t)) || T(t)!=T(t))
    {
      T(t) = 0;
    }
  }
  DerivedS SB = S(B,igl::placeholders::all);
  Eigen::VectorXi MB = B.array()+1;
  for(int b = 0;b<MB.size();b++)
  {
    if(MB(b) >= S.rows())
    {
      MB(b) = S.rows()-1;
    }
  }
  DerivedS SMB = S(MB,igl::placeholders::all);
  Q = SB.array() + ((SMB.array()-SB.array()).colwise()*T.array());

  // Remove extra point at end
  J.conservativeResize(J.size()-1);
}

#ifdef IGL_STATIC_LIBRARY
// Explicit template instantiation
// generated by autoexplicit.sh
template void igl::ramer_douglas_peucker<Eigen::Matrix<double, -1, -1, 0, -1, -1>, Eigen::Matrix<double, -1, -1, 0, -1, -1>, Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<double, -1, -1, 0, -1, -1> >(Eigen::MatrixBase<Eigen::Matrix<double, -1, -1, 0, -1, -1> > const&, Eigen::Matrix<double, -1, -1, 0, -1, -1>::Scalar, Eigen::PlainObjectBase<Eigen::Matrix<double, -1, -1, 0, -1, -1> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<double, -1, -1, 0, -1, -1> >&);
// generated by autoexplicit.sh
template void igl::ramer_douglas_peucker<Eigen::Matrix<double, -1, 2, 0, -1, 2>, Eigen::Matrix<double, -1, 2, 0, -1, 2>, Eigen::Matrix<int, -1, 1, 0, -1, 1>, Eigen::Matrix<double, -1, 2, 0, -1, 2> >(Eigen::MatrixBase<Eigen::Matrix<double, -1, 2, 0, -1, 2> > const&, Eigen::Matrix<double, -1, 2, 0, -1, 2>::Scalar, Eigen::PlainObjectBase<Eigen::Matrix<double, -1, 2, 0, -1, 2> >&, Eigen::PlainObjectBase<Eigen::Matrix<int, -1, 1, 0, -1, 1> >&, Eigen::PlainObjectBase<Eigen::Matrix<double, -1, 2, 0, -1, 2> >&);
#endif