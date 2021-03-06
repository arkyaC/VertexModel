#include <random>
#include <iostream>
#include <fstream>
#include <cmath>
#include <stdlib.h>

//#define N_steps 40000
#define N_steps 10000
#define N_particles 64
using namespace std;

/*
sizes of particles, i.e. R_eq are selected from a uniform distribution
----------------------RUN CODE WITH COMMAND LINE INPUTS density AND noise IN THAT ORDER-------------------
*/
int main(int argc, char const *argv[]) {
  double noise, rho;
	if(argc==1)
    noise = 0.6, rho = 0.3; //density, noise
  else if(argc!=3){
    cout<<"Invalid arguments! Must enter both noise and density parameter"<<endl;
    return 0;
  }
  else{
    rho = atof(argv[1]);
    noise = atof(argv[2]);
  }

	double pi = 4*atan(1.0);

  double N = N_particles;
  double mu = 1.0, tau = 1.0;
  double Req_mean = 5.0/6, R0_mean = 1.0; //radius parameters
  double v0 = 1; //self-propelling velocity
  double polydisp = 0.1; //fraction of polydispersity

  double Fadh = 0.75, Frep = 30.0; //force parameters

  double L = R0_mean * sqrt(N/rho); //length of a side set using the given density
  double delX = L * sqrt(2.0/N);
  double delY = delX;
  
  ofstream L_val;
  L_val.open("../data/zebra/L_val.txt");
  L_val<<L;
  L_val.close();


  random_device rd;  //Will be used to obtain a seed for the random number engine
  mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
  uniform_real_distribution<> dis(0,1); //uniform distribution on [0,1)

  double* Req = new double[N_particles];
  double* R0 = new double[N_particles];
  double* xpos = new double[N_particles];
  double* ypos = new double[N_particles];
  double* theta = new double[N_particles];

//---------------------------initializing sol matrices-----------------------------
	int ctr = 0;
	int M = (int)sqrt(N_particles);
  for (int i=0;i<N_particles;i++){
    xpos[i] = delX/2 + delX*ctr; //x coordinate
    ctr = (ctr+1) % M;
  }
  ctr = -1;
  for (int i=0;i<N_particles;i++){
  	if (i%M==0)
    	ctr++;
    ypos[i] = delY/2 + delY*ctr; //y coordinate
  }
  for (int i=0;i<N_particles;i++){
  	theta[i] = 2 * pi * dis(gen) - pi; //direction of self-propulsion
    Req[i] = Req_mean + polydisp * Req_mean * (dis(gen) - 0.5);
    R0[i] = (6.0/5) * Req[i];
    cout<<i<<"\t Req = "<<Req[i]<<"\tR0 = "<<R0[i]<<endl;
  }

  double delT = 0.005*R0_mean/v0;
  double noiseAmp = noise/sqrt(delT);

  double** solX = new double*[N_steps+1];
  for(int i=0;i<N_steps+1;i++){
    solX[i] = new double[N_particles];
  }
  double** solY = new double*[N_steps+1];
  for(int i=0;i<N_steps+1;i++){
    solY[i] = new double[N_particles];
  }
  double** solTheta = new double*[N_steps+1];
  for(int i=0;i<N_steps+1;i++){
    solTheta[i] = new double[N_particles];
  }

  //double* order = new double[N_steps];//order parameter of the kth step
  
  for (int i = 0;i<N_particles;i++){
    solX[0][i] = xpos[i];//Initialize 0th step
    solY[0][i] = ypos[i];
    solTheta[0][i] = theta[i];
  }

	ofstream dump_pos;
	dump_pos.open("../data/zebra/movie_dump.txt");

	ctr = 0;
  double* rhsX = new double[N_particles];
  double* rhsY = new double[N_particles];
  double* rhsTheta = new double[N_particles];

	float progress = 0.0;
	int progBarWidth = 60;
  //---------------------------main solver loop-----------------------------------
  for(int k = 0;k<N_steps;k++){
		//progress bar
		if(k%100==0){
			progress = (k*1.0)/N_steps;
			int pos = progBarWidth * progress;
			cout<<"\r"<<(progress*100)<<"% complete: ";
			cout<<string(pos,'|');
			cout.flush();
		}

		//interaction forces evaluation
    for (int i = 0;i<N_particles;i++){
      double interaxn[2] = {0,0}; //interaction force vector for ith particle
      for (int j = 0;j<N_particles;j++){
        if (i==j)
          continue;
        double dx = solX[k][i] - solX[k][j];//dij vector = (dx,dy)
        double dy = solY[k][i] - solY[k][j];
        if (abs(dx) > 0.5*L)
          dx = dx - copysign(L,dx);
        if (abs(dy) > 0.5*L)
          dy = dy - copysign(L,dy);
        double dijSq = dx*dx + dy*dy;
        double dij = sqrt(dijSq);
        double eij[2] = {dx/dij,dy/dij};
        if (dijSq<Req[i]*Req[i]){
            interaxn[0] -= Frep*(dij-Req[i])/Req[i] * eij[0];
            interaxn[1] -= Frep*(dij-Req[i])/Req[i] * eij[1];
          }
        else if (dijSq>=Req[i]*Req[i] && dijSq<R0[i]*R0[i]){
            interaxn[0] -= Fadh*(dij-Req[i])/(R0[i]-Req[i]) * eij[0];
            interaxn[1] -= Fadh*(dij-Req[i])/(R0[i]-Req[i]) * eij[1];
          }
      }
      interaxn[0] *= mu;
      interaxn[1] *= mu;
      rhsX[i] = v0*cos(solTheta[k][i])+interaxn[0];
      rhsY[i] = v0*sin(solTheta[k][i])+interaxn[1];
    }
    //double avgvel[2] = {0,0};
    for (int i = 0;i<N_particles;i++){
      double ni[2] = {cos(solTheta[k][i]),sin(solTheta[k][i])};
      double vi[2] = {rhsX[i],rhsY[i]};
      double normvi = sqrt(pow(vi[0],2)+pow(vi[1],2));
      vi[0] = vi[0]/normvi;
      vi[1] = vi[1]/normvi;
      //avgvel[0] += vi[0];
      //avgvel[1] += vi[1];
      rhsTheta[i] = asin(ni[0]*vi[1]-ni[1]*vi[0]);
      solX[k+1][i] = solX[k][i] + delT*rhsX[i];
      solX[k+1][i] -= L*floor(solX[k+1][i]/L);//wrapping around excesses
      solY[k+1][i] = solY[k][i] + delT*rhsY[i];
      solY[k+1][i] -= L*floor(solY[k+1][i]/L);//wrapping around excesses
      solTheta[k+1][i] = solTheta[k][i] + delT*(rhsTheta[i]/tau + noiseAmp * (dis(gen)-0.5));
			if(k%10==0){
				if(i==0)
					ctr++;
				
        dump_pos<<ctr<<"\t"<<i<<"\t"<<solX[k][i]<<"\t"<<solY[k][i]<<"\t"<<cos(solTheta[k][i])<<"\t"<<sin(solTheta[k][i])<<endl;
        //dump_pos<<ctr<<"\t"<<i<<"\t"<<solX[k][i]<<"\t"<<solY[k][i]<<"\t"<<rhsX[i]<<"\t"<<rhsY[i]<<endl;
			}
    }
    //order[k] = sqrt(pow(avgvel[0],2)+pow(avgvel[1],2))/N_particles;

  }
	dump_pos.close();
	cout<<"\n Done"<<endl;

  /*
  ofstream dump_data;
  dump_data.open("../data/zebra/order_dump.txt");
  for (int i=0;i<N_steps;i++){
    dump_data<<(i+1)<<"\t"<<order[i]<<"\n";
  }
  dump_data.close();
  */
  return 0;
}
