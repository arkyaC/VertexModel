//------------------incomplete-----------------
#include <iostream>
#include <fstream>
#include <random>
#include <cmath>
#define N_steps 10000
#define ensmblMax 50

using namespace std;

int main(int argc, char const *argv[]){
  int N_beads=50;
  solver(N_beads);
  return 0;
}

void solver(int beads) {
  const int N_beads = beads;

  int N_eff = N_beads + 2;
  double L = 1.0; //length of chain
  double D = 1.0, k_eff = 1.0;//D=diffusion coefficient; define k_eff=k/zeta, zeta being the drag coefficient
  double delT = .1;

  double* xpos = new double[N_eff];
  double* ypos = new double[N_eff];

  random_device rd;
  mt19937 gen(rd());//seeding
  normal_distribution<> dis(0.0,1.0);//Gaussian random number

  for (int i=1;i<=N_beads;i++){
    xpos[i] = (L/(N_beads-1))*(i-1); //initially on a 1D lattice
    ypos[i] = 0;
  }
  xpos[0] = xpos[1]; ypos[0] = ypos[1];//"ghost" nodes
  xpos[N_beads+1] = xpos[N_beads]; ypos[N_beads+1] = ypos[N_beads];
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

  double* comX = new double[N_steps+1];
  double* comY = new double[N_steps+1];
  double* delcomX = new double[N_steps+1];
  double* delcomY = new double[N_steps+1];
  double* comSq = new double[N_steps+1];

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

    //comX[k] = 0.0; comY[k] = 0.0;

    for(int i=1;i<=N_beads;i++){
      double rhs_i = (-1)*(k_eff)*(2*solX[k][i] - solX[k][i+1] - solX[k][i-1]) ;
      solX[k+1][i] = solX[k][i] + rhs_i * delT + dis(gen)*sqrt(4*D*delT);
      delcomX[k] += solX[k+1][i] - solX[k][i];

      rhs_i = (-1)*(k_eff)*(2*solY[k][i] - solY[k][i+1] - solY[k][i-1]);
      solY[k+1][i] = solY[k][i] + rhs_i * delT + dis(gen)*sqrt(4*D*delT);
      delcomY[k] += solY[k+1][i] - solY[k][i];

      //delcomX[k] += solX[k+1][i] - solX[k][i]; //evaluating position of
      //delcomY[k] += solY[k+1][i] - solY[k][i]; //centre of mass
    }
    comX[k+1] = comX[k] + delcomX[k]/N_beads;
    comY[k+1] = comY[k] + delcomY[k]/N_beads;
    comSq[k] = pow(comX[k],2) + pow(comY[k],2);
    //boundary conditions
    solX[k+1][0] = solX[k+1][1]; solX[k+1][N_beads+1] = solX[k+1][N_beads];
    solY[k+1][0] = solY[k+1][1]; solY[k+1][N_beads+1] = solY[k+1][N_beads];
  }
  cout<<endl;

  ofstream dump_com;
  dump_com.open("com_dump.txt");
  for (int i=0;i<N_steps;i++){
    //dump_com<<(i)<<"\t"<<comSq[i]<<"\n";//steps 0,1,2...
    if (i%10==0){
      //dump_com<<(i+1)<<"\t"<<comSq[i]<<"\n";//steps 0,1,2...
      dump_com<<(i+1)<<"\t"<<comSq[i]<<"\n";
      //cout<<comSq[i]<<endl;
    }
  }
  dump_com.close();
}
