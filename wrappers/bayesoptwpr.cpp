#include "bayesoptwpr.h"
#include "bayesoptcont.hpp"      


inline void copyCarrayInVector(const double* array, int n, vectord& vec)
{
  for(int i = 0; i<n;i++)
    vec(i) = array[i];
}


inline void copyVectorInArray(double* array, int n, const vectord& vec)
{
  for(int i = 0; i<n;i++)
    array[i] = vec(i);
}


class CSKO: public SKO 
{
 public:

  CSKO( NonParametricProcess* gp = NULL): 
    SKO(gp)
  {}; 


  double evaluateSample( const vectord &Xi ) 
  {
    int n = static_cast<int>(Xi.size());
    double *x = new double[n];

    copyVectorInArray(x,n,Xi);
    double result = mF(n,x,NULL,mOtherData);
    delete[] x;

    return result;
  };

  void set_eval_funct(eval_func f)
  {
    mF = f;
  }


  void save_other_data(void* other_data)
  {
    mOtherData = other_data;
  }

 protected:

  void* mOtherData;
  eval_func mF;
};



int bayes_optimization(int nDim, eval_func f, void* f_data,
		       const double *lb, const double *ub, /* bounds */
		       double *x, /* in: initial guess, out: minimizer */
		       double *minf, /* out: minimum */
		       int maxiniteval, int maxeval, 
		       gp_params params,
		       criterium_name c_name,
		       surrogate_name gp_name,
		       kernel_name k_name)
{

  vectord result(nDim);

  vectord lowerBound(nDim); 
  vectord upperBound(nDim); 

  copyCarrayInVector(lb,nDim,lowerBound);
  copyCarrayInVector(ub,nDim,upperBound);

  NonParametricProcess* gp;

  switch(gp_name)
    {
    case s_gaussianProcess: 
      gp = new BasicGaussianProcess(params.theta,params.noise);
      break;

    case s_gaussianProcessHyperPriors:
      gp = new GaussianProcess(params.theta,params.noise,
			       params.alpha,params.beta,
			       params.delta);
      break;

    case s_studentTProcess:
      gp = new StudentTProcess(params.theta,params.noise);
      break;

    case s_error:
    default: 
      std::cout << "Surrogate function not supported" << std::endl;
      return -1;
    }

  gp->setKernel(k_name);

  CSKO optimizer(gp);

  optimizer.setCriteria(c_name);
  optimizer.set_eval_funct(f);
  optimizer.save_other_data(f_data);
  optimizer.setInitSet(maxiniteval);
  optimizer.optimize(result,lowerBound,upperBound,maxeval);

  copyVectorInArray(x,nDim,result);

  return 1; /* everything ok*/
  
}