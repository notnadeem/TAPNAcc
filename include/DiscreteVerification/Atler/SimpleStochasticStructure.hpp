#ifndef SIMPLE_STOCHASTIC_STRUCTURE
#define SIMPLE_STOCHASTIC_STRUCTURE

// TODO: Check if this is correct

#include <random>
/*#include <sstream>*/
#include <string>

namespace VerifyTAPN::Atler::SimpleSMC {

enum FiringMode { Oldest, Youngest, Random };

std::string firingModeName(FiringMode type);

enum DistributionType { Constant, Uniform, Exponential, Normal, Gamma, Erlang, DiscreteUniform, Geometric };

std::string distributionName(DistributionType type);

struct SMCUniformParameters {
  double a;
  double b;
};
struct SMCExponentialParameters {
  double rate;
};
struct SMCNormalParameters {
  double mean;
  double stddev;
};
struct SMCConstantParameters {
  double value;
};
struct SMCGammaParameters {
  double shape;
  double scale;
};
struct SMCDiscreteUniformParameters {
  int a;
  int b;
};
struct SMCGeometricParameters {
  double p;
};

union DistributionParameters {
  SMCUniformParameters uniform;
  SMCExponentialParameters exp;
  SMCNormalParameters normal;
  SMCConstantParameters constant;
  SMCGammaParameters gamma;
  SMCDiscreteUniformParameters discreteUniform;
  SMCGeometricParameters geometric;
};

struct Distribution {
  DistributionType type;
  DistributionParameters parameters;

  template <typename T> double sample(T &engine, const unsigned int precision = 0) const {
    double date = 0;
    switch (type) {
    case Constant:
      date = parameters.constant.value;
      break;
    case Uniform:
      date = std::uniform_real_distribution<double>(parameters.uniform.a, parameters.uniform.b)(engine);
      break;
    case Exponential:
      date = std::exponential_distribution<double>(parameters.exp.rate)(engine);
      break;
    case Normal:
      date = std::normal_distribution<double>(parameters.normal.mean, parameters.normal.stddev)(engine);
      break;
    case Gamma:
    case Erlang:
      date = std::gamma_distribution<double>(parameters.gamma.shape, parameters.gamma.scale)(engine);
      break;
    case DiscreteUniform:
      date = static_cast<double>(
          std::uniform_int_distribution<int>(parameters.discreteUniform.a, parameters.discreteUniform.b)(engine));
      break;
    case Geometric:
      date = date = static_cast<double>(std::geometric_distribution<int>(parameters.geometric.p)(engine));
      break;
    }
    if (precision > 0) {
      double factor = pow(10.0, precision);
      date = round(date * factor) / factor;
    }
    return std::max(date, 0.0);
  }

  static Distribution urgent();

  static Distribution defaultDistribution();

  static Distribution fromParams(int distrib_id, double param1, double param2);

  std::string toXML() const;
};

} // namespace VerifyTAPN::Atler::SimpleSMC

#endif
