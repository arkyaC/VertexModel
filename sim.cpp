//------------------don't modify delT carelessly, don't lower it from current value-----------------

//FINAL SIMULATION  !!!!!!!!!!!!!!!!INCOMPLETE!!!!!!!!!!!!!
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#define N_steps 5000

using namespace std;

random_device rd;
mt19937 gen(rd());//seeding
normal_distribution<> dis(0.0,1.0);//Gaussian random number

int main(int argc, char const *argv[]) {
  const int N_beads = 5;

  int N_eff = N_beads + 2;
  double b = sqrt(2.0), kbT = 1.0, zeta = 1.0;//zeta is drag coefficient
  double k_sp = 2*kbT/(b*b); //spring constant
  double D = kbT/zeta;

  double delT = .1;

  //-----------------------Initial shape of the polymer---------------------
  double* xpos = new double[N_eff];
  double* ypos = new double[N_eff];

  xpos[1] = 0.0; ypos[1] = 0.0;
  for (int i=2;i<=N_beads;i++){
    xpos[i] = xpos[i-1] + b*dis(gen); //initially a 
    ypos[i] = ypos[i-1] + b*dis(gen); //gaussian chain
  }
  xpos[0] = xpos[1]; ypos[0] = ypos[1];//"ghost" nodes
  xpos[N_beads+1] = xpos[N_beads]; ypos[N_beads+1] = ypos[N_beads];

  //-------------------------Polymer dynamics--------------------------------
  //Declare solution matrices
  double** solX = new double*[N_steps+1];
  for(int i=0;i<N_steps+1;i++){
    solX[i] = new double[N_eff];
  }
  double** solY = new double*[N_steps+1];
  for(int i=0;i<N_steps+1;i++){
    solY[i] = new double[N_eff];
  }
  //Initialize 0th step
  for (int i = 0;i<N_eff;i++){
    solX[0][i] = xpos[i];
    solY[0][i] = ypos[i];
  }

  //progress bar
  float progress = 0.0;
	int progBarWidth = 60;
  for(int k = 0;k<N_steps;k++){
    //progress bar
		if(k%100==0){
			progress = (k*1.0)/N_steps;
			int pos = progBarWidth * progress;
			cout<<"\r"<<(progress*100)<<"% complete: ";
			cout<<string(pos,'|');
			cout.flush();
		}

    for(int i=1;i<=N_beads;i++){ //nearest neighbour interactions
      double rhs_i = (-1)*(k_sp/zeta)*(2*solX[k][i] - solX[k][i+1] - solX[k][i-1]) ;
      solX[k+1][i] = solX[k][i] + rhs_i * delT + dis(gen)*sqrt(2*D*delT);

      rhs_i = (-1)*(k_sp/zeta)*(2*solY[k][i] - solY[k][i+1] - solY[k][i-1]);
      solY[k+1][i] = solY[k][i] + rhs_i * delT + dis(gen)*sqrt(2*D*delT);
    }
    //boundary conditions
    solX[k+1][0] = solX[k+1][1]; solX[k+1][N_beads+1] = solX[k+1][N_beads];
    solY[k+1][0] = solY[k+1][1]; solY[k+1][N_beads+1] = solY[k+1][N_beads];
  }
  cout<<endl; //for the progress bar

  return 0;
}
