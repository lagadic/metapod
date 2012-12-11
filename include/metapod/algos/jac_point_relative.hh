// Copyright 2012,
//
// Antonio El Khoury (JRL/LAAS, CNRS/AIST)
// Sébastien Barthélémy (Aldebaran Robotics)
//
// This file is part of metapod.
// metapod is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// metapod is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// You should have received a copy of the GNU Lesser General Public License
// along with metapod.  If not, see <http://www.gnu.org/licenses/>.

/// Implementation of articular jacobian of point given an influencing
/// chain of joints.

#ifndef METAPOD_JAC_POINT_RELATIVE_HH
# define METAPOD_JAC_POINT_RELATIVE_HH

# include <metapod/tools/common.hh>
# include <metapod/tools/deepest_common_ancestor.hh>
# include <metapod/tools/backward_traversal.hh>

namespace metapod
{

  // convert point coordinates from Body frame to world frame.
  // The result is valid if bcacl has been called to update the forward
  // kinematics model.
  template< typename Body >
  struct convert_to_world
  {
    static Vector3d run(const Vector3d & b_p)
    {
      return Body::iX0.applyInv(b_p);
    }
  };

  // specialization for the NP body: the point is already in the proper
  // coordinate frame, and the generic computation would not compile.
  template<>
  struct convert_to_world<NP>
  {
    static Vector3d run(const Vector3d & b_p)
    {
      return b_p;
    }
  };

  /// \addtogroup jac_point_relative Point in Chain Articular Jacobian Algorithm
  ///
  /// Compute the articular jacobian J which gives the world velocity of a point
  /// (given in body coordinates) attached to a body (called EndNode) relative
  /// to another body called StartNode.
  /// Obvioulsy, the only non-zero blocks in the jacobian will correspond to
  /// joints in the kinematic chain between StartNode and EndNode.
  //
  /// Such a jacobian can be used to control the relative pose of a body
  /// with respect to another body.
  ///
  /// The jacobian size is normally 6 x Robot:NBDOF, however this algorithm
  /// takes an optional "offset" argument, and the actual jacobian size is
  /// 6 x (Robot:NBDOF+offset)
  /// If offset is positive, the first "offset" columns won't be touched by the
  /// algorithm.
  /// If offset is negative, the algorithm will only compile if the first
  /// offset column are not written by the algorithm, that is, if the
  /// correspondig joints are not in the kinematic chain between StartNode
  /// and EndNode.
  ///
  /// \{

  // Note: we might want to drop the support of the offset parameter and let
  // the user pass a sub-block as the jacobian argument. Alas that would
  // support negative offsets.
  template< typename Robot, typename StartNode, typename EndNode,
            int offset = 0, bool call_bcalc = true >
  struct jac_point_relative {
    typedef Eigen::Matrix< FloatType, 6, Robot::NBDOF + offset> jacobian_t;
    typedef typename deepest_common_ancestor<StartNode, EndNode>::type AncestorNode;
    typedef EndNode EndBody;

    template <typename Node> struct StartToAncestorVisitor
    {
        typedef Node Body;
        static void discover(const Vector3d & p,
                             jacobian_t & J)
        {
          // Compute jacobian block for current node. Formula is given by:
          // Ji = - pX0 * (iX0)^(-1) * Si,
          // where pX0 is the word transform in the point frame,
          // iX0 is the world transform in the ith body frame,
          // Si is the ith joint motion subspace matrix.
          J.template
              block<6, Node::Joint::NBDOF>
              (0, Node::Joint::positionInConf + offset) = -
	    Body::iX0.inverse ().toPointFrame (p).apply(Node::Joint::S);
        }
        static void finish(const Vector3d &,
                           jacobian_t &) {};
    };

    template <typename Node> struct EndToAncestorVisitor
    {
        typedef Node Body;
        static void discover(const Vector3d & p,
                             jacobian_t & J)
        {
          // Compute jacobian block for current node. Formula is given by:
          // Ji = pX0 * (iX0)^(-1) * Si,
          // where pX0 is the word transform in the point frame,
          // iX0 is the world transform in the ith body frame,
          // Si is the ith joint motion subspace matrix.
          J.template
            block<6, Node::Joint::NBDOF>
            (0,  Node::Joint::positionInConf + offset) =
	    //      Node::Joint::applyToS(Body::iX0.inverse ().toPointFrame (p));
	    Body::iX0.inverse ().toPointFrame (p).apply( Node::Joint::S);
        }
        static void finish(const Vector3d &,
                           jacobian_t &) {};
    };

    /// \brief Compute the articular jacobian J.
    ///
    /// \param q Configuration vector: it is used to update all body
    /// spatial transforms if bcalc is equal to true.
    ///
    /// \param e_p Coordinates of point in EndBody coordinates.
    //
    /// \retval J Computed jacobian of size 6 x(NBDOF+offset). The algorithm
    /// will only write to the non-zero blocks of the jacobian. User should
    /// presumably zero J before passing it to the algorithm for the first
    /// time.
    static void run(const typename Robot::confVector & q,
                    const Vector3d & e_p,
                    jacobian_t & J)
    {
      // Update body transformations.
      if (call_bcalc)
      {
        bcalc < Robot >::run(q);
      }

      // Compute point coordinates in world frame.h
      // We need to delegate this computation to account for the cases
      // where EndNode == NP.
      Vector3d p = convert_to_world<EndNode>::run(e_p);

      backward_traversal< StartToAncestorVisitor, Robot,
          StartNode, AncestorNode>::run(p, J);
      backward_traversal< EndToAncestorVisitor, Robot,
          EndNode, AncestorNode>::run(p, J);
    }
  };

} // end of namespace metapod.

#endif
