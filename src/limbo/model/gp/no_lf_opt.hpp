#ifndef LIMBO_MODEL_GP_NO_LF_OPT_HPP
#define LIMBO_MODEL_GP_NO_LF_OPT_HPP

namespace limbo {
    namespace model {
        namespace gp {
            ///@ingroup model_opt
            ///do not optimize anything
            template <typename Params>
            struct NoLFOpt {
                template <typename GP>
                void operator()(GP&) const {}
            };
        }
    }
}

#endif
