/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010-2014 Joel Andersson, Joris Gillis, Moritz Diehl,
 *                            K.U. Leuven. All rights reserved.
 *    Copyright (C) 2011-2014 Greg Horn
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


#include "reshape.hpp"
#include "../std_vector_tools.hpp"
#include "../matrix/matrix_tools.hpp"
#include "mx_tools.hpp"
#include "../sx/sx_tools.hpp"
#include "../function/sx_function.hpp"

using namespace std;

namespace casadi {

  Reshape::Reshape(const MX& x, Sparsity sp) {
    casadi_assert(x.nnz()==sp.nnz());
    setDependencies(x);
    setSparsity(sp);
  }

  Reshape* Reshape::clone() const {
    return new Reshape(*this);
  }

  void Reshape::evalD(cp_double* input, p_double* output, int* itmp, double* rtmp) {
    evalGen<double>(input, output, itmp, rtmp);
  }

  void Reshape::evalSX(cp_SXElement* input, p_SXElement* output,
                       int* itmp, SXElement* rtmp) {
    evalGen<SXElement>(input, output, itmp, rtmp);
  }

  template<typename T>
  void Reshape::evalGen(const T* const* arg, T* const* res,
                        int* itmp, T* rtmp) {
    if (arg[0]!=res[0]) copy(arg[0], arg[0]+nnz(), res[0]);
  }

  void Reshape::spFwd(cp_bvec_t* arg,
                      p_bvec_t* res, int* itmp, bvec_t* rtmp) {
    copyFwd(arg[0], res[0], nnz());
  }

  void Reshape::spAdj(p_bvec_t* arg,
                      p_bvec_t* res, int* itmp, bvec_t* rtmp) {
    copyAdj(arg[0], res[0], nnz());
  }

  void Reshape::printPart(std::ostream &stream, int part) const {
    // For vectors, reshape is also a transpose
    if (dep().isVector(true) && sparsity().isVector(true)) {
      // Print as transpose: X'
      if (part!=0) {
        stream << "'";
      }
    } else {
      // Print as reshape(X) or vec(X)
      if (part==0) {
        if (sparsity().isVector()) {
          stream << "vec(";
        } else {
          stream << "reshape(";
        }
      } else {
        stream << ")";
      }
    }
  }

  void Reshape::evalMX(const std::vector<MX>& arg, std::vector<MX>& res) {
    res[0] = reshape(arg[0], shape());
  }

  void Reshape::evalFwd(const std::vector<std::vector<MX> >& fseed,
                        std::vector<std::vector<MX> >& fsens) {
    for (int d = 0; d<fsens.size(); ++d) {
      fsens[d][0] = reshape(fseed[d][0], shape());
    }
  }

  void Reshape::evalAdj(const std::vector<std::vector<MX> >& aseed,
                        std::vector<std::vector<MX> >& asens) {
    for (int d=0; d<aseed.size(); ++d) {
      asens[d][0] += reshape(aseed[d][0], dep().shape());
    }
  }

  void Reshape::generate(std::ostream &stream, const std::vector<int>& arg,
                                  const std::vector<int>& res, CodeGenerator& gen) const {
    if (arg[0]==res[0]) return;
    gen.copyVector(stream, gen.work(arg[0]), nnz(), gen.work(res[0]), "i", false);
  }

  MX Reshape::getReshape(const Sparsity& sp) const {
    return reshape(dep(0), sp);
  }

  MX Reshape::getTranspose() const {
    // For vectors, reshape is also a transpose
    if (dep().isVector(true) && sparsity().isVector(true)) {
      return dep();
    } else {
      return MXNode::getTranspose();
    }
  }

  bool Reshape::isValidInput() const {
    if (!dep()->isValidInput()) return false;
    return true;
  }

  int Reshape::numPrimitives() const {
    return dep()->numPrimitives();
  }

  void Reshape::getPrimitives(std::vector<MX>::iterator& it) const {
    dep()->getPrimitives(it);
  }

  void Reshape::splitPrimitives(const MX& x, std::vector<MX>::iterator& it) const {
    dep()->splitPrimitives(reshape(x, dep().shape()), it);
  }

  MX Reshape::joinPrimitives(std::vector<MX>::const_iterator& it) const {
    return reshape(dep()->joinPrimitives(it), shape());
  }

  bool Reshape::hasDuplicates() {
    return dep()->hasDuplicates();
  }

  void Reshape::resetInput() {
    dep()->resetInput();
  }

} // namespace casadi
