//| Copyright Inria May 2015
//| This project has received funding from the European Research Council (ERC) under
//| the European Union's Horizon 2020 research and innovation programme (grant
//| agreement No 637972) - see http://www.resibots.eu
//|
//| Contributor(s):
//|   - Jean-Baptiste Mouret (jean-baptiste.mouret@inria.fr)
//|   - Antoine Cully (antoinecully@gmail.com)
//|   - Kontantinos Chatzilygeroudis (konstantinos.chatzilygeroudis@inria.fr)
//|   - Federico Allocati (fede.allocati@gmail.com)
//|   - Vaios Papaspyros (b.papaspyros@gmail.com)
//|   - Roberto Rama (bertoski@gmail.com)
//|
//| This software is a computer library whose purpose is to optimize continuous,
//| black-box functions. It mainly implements Gaussian processes and Bayesian
//| optimization.
//| Main repository: http://github.com/resibots/limbo
//| Documentation: http://www.resibots.eu/limbo
//|
//| This software is governed by the CeCILL-C license under French law and
//| abiding by the rules of distribution of free software.  You can  use,
//| modify and/ or redistribute the software under the terms of the CeCILL-C
//| license as circulated by CEA, CNRS and INRIA at the following URL
//| "http://www.cecill.info".
//|
//| As a counterpart to the access to the source code and  rights to copy,
//| modify and redistribute granted by the license, users are provided only
//| with a limited warranty  and the software's author,  the holder of the
//| economic rights,  and the successive licensors  have only  limited
//| liability.
//|
//| In this respect, the user's attention is drawn to the risks associated
//| with loading,  using,  modifying and/or developing or reproducing the
//| software by the user in light of its specific status of free software,
//| that may mean  that it is complicated to manipulate,  and  that  also
//| therefore means  that it is reserved for developers  and  experienced
//| professionals having in-depth computer knowledge. Users are therefore
//| encouraged to load and test the software's suitability as regards their
//| requirements in conditions enabling the security of their systems and/or
//| data to be ensured and,  more generally, to use and operate it in the
//| same conditions as regards security.
//|
//| The fact that you are presently reading this means that you have had
//| knowledge of the CeCILL-C license and that you accept its terms.
//|
#ifndef LIMBO_KERNEL_SQUARED_EXP_ARD_HPP
#define LIMBO_KERNEL_SQUARED_EXP_ARD_HPP

#include <Eigen/Core>
#include <limbo/tools.hpp>

namespace limbo {
    namespace defaults {
        struct kernel_squared_exp_ard {
            /// @ingroup kernel_defaults
            BO_PARAM(int, k, 0); //equivalent to the standard exp ARD
            /// @ingroup kernel_defaults
            BO_PARAM(double, sigma_sq, 1);
        };
    }

    namespace kernel {
        /**
        @ingroup kernel
        \rst

        Squared exponential covariance function with automatic relevance detection (to be used with a likelihood optimizer)
        Computes the squared exponential covariance like this:

        .. math::
            k_{SE}(x, y) = \sigma^2 \exp \Big(-\frac{1}{2}(x-y)^TM(x-y)\Big),

	 with :math:`M = \Lambda\Lambda^T + diag(l_1^{-2}, \dots, l_n^{-2})` being the characteristic length scales and :math:`\alpha` describing the variability of the latent function. The parameters :math:`l_1^2, \dots, l_n^2, \Lambda` are expected in this order in the parameter array. :math:`\Lambda` is a :math:`D\times k` matrix with :math:`k<D`.

        Parameters:
           - ``double sigma_sq`` (signal variance)
           - ``int k`` (number of columns of :math:`\Lambda` matrix)

        Reference: :cite:`Rasmussen2006`, p. 106 & :cite:`brochu2010tutorial`, p. 10
        \endrst
        */
        template <typename Params>
        struct SquaredExpARD {
            SquaredExpARD(int dim = 1) : _sf2(0), _ell(dim), _input_dim(dim)
            {
                Eigen::VectorXd p = Eigen::VectorXd::Zero(_ell.size() + 1);
                this->set_h_params(p);
            }

            size_t h_params_size() const { return _ell.size() + 1; }

            // Return the hyper parameters in log-space
            Eigen::VectorXd h_params() const { 
                Eigen::VectorXd result(_input_dim + 1);
                result.segment(0, _input_dim) = _ell.array().log();
                result(_input_dim) = std::log(_sf2);
                return result;
            }

            // We expect the input parameters to be in log-space
            void set_h_params(const Eigen::VectorXd& p)
            {
                _h_params = p;
                // _ell = p.segment(0, _input_dim).array().exp();
                for (size_t i = 0; i < _input_dim; ++i) _ell(i) = std::exp(p(i));
                _sf2 = std::exp(2 * p(_input_dim)); //std in log-space
            }

            Eigen::VectorXd grad(const Eigen::VectorXd& x1, const Eigen::VectorXd& x2) const
            {
                Eigen::VectorXd grad(_input_dim + 1);
                Eigen::VectorXd z = (x1 - x2).cwiseQuotient(_ell).array().square();
                double k = _sf2 * std::exp(-0.5 * z.sum());
                grad.head(_input_dim) = z * k;
                grad.tail(1) = limbo::tools::make_vector(2 * k);
                return grad;
            }

            double operator()(const Eigen::VectorXd& x1, const Eigen::VectorXd& x2) const
            {
                if (x1.size() != _ell.size())
                    std::cout << x1.size() << " " << _ell.size() << std::endl;
                assert(x1.size() == _ell.size());
                double z = (x1 - x2).cwiseQuotient(_ell).squaredNorm();
                return _sf2 * std::exp(-0.5 * z);
            }

            const Eigen::VectorXd& ell() const { return _ell; }

        protected:
            double _sf2;
            Eigen::VectorXd _ell;
            size_t _input_dim;
            Eigen::VectorXd _h_params;
        };
    }
}

#endif
