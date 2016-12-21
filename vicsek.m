numberOfPoints = 300;
rho = 0.5;
L = sqrt(numberOfPoints/rho);
noise = 0.1;
v = 0.03; %as stated in the paper (for optimum results)
r = 1; %definition of neighbourhood for averaging
x =L* rand(1, 10000);
y =L* rand(1, 10000);
minAllowableDistance = 0.1; %arbitrary (just for better visualisation)
% Initialize first point.
keeperX = x(1);
keeperY = y(1);
% Try dropping down more points.
counter = 1;
k=1;
while counter<=numberOfPoints
	% Get a trial point.
	thisX = x(k);
	thisY = y(k);
	% See how far is is away from existing keeper points.
	distances = sqrt((thisX-keeperX).^2 + (thisY - keeperY).^2);
	minDistance = min(distances);
    if minDistance >= minAllowableDistance
        keeperX(counter) = thisX;
        keeperY(counter) = thisY;
        counter = counter + 1;
    end
    k=k+1;
end

Nsteps=2000;
theta = 2*pi*rand(1,numberOfPoints);
timedelta = 1; %arbitrary
y = zeros(Nsteps+1,3*numberOfPoints);
y(1,:) = [keeperX,keeperY,theta];
orderParam = 0; %to be evaluated later

for k=1:Nsteps
	posX=y(k,1:numberOfPoints); % x position matrix
	posY=y(k,numberOfPoints+1:2*numberOfPoints); % y position matrix
	theta=y(k,2*numberOfPoints+1:end);
    newTheta=theta;
	vel=[v*cos(theta) v*sin(theta)];
    rhs = zeros(1,3*numberOfPoints);
    for i=1:numberOfPoints %evaluating updated angle parameter
        avg = 0;
        ctr = 0;
        for j=1:numberOfPoints
            distIJ = sqrt((posX(j)-posX(i))^2 + (posY(j) - posY(i))^2);
            if distIJ<r
                avg = avg + theta(j);
                ctr = ctr + 1;
            end
        end
        avg = avg/ctr;
        newTheta(i) = avg + (noise*rand - noise/2);
    end
    posX = posX + vel(1:numberOfPoints)*timedelta;
    posY = posY + vel(numberOfPoints+1:end)*timedelta;
    posX = L*(posX/L-floor(posX/L));
	posY = L*(posY/L-floor(posY/L));
    theta = newTheta;
    y(k+1,:)=[posX,posY,theta];%Euler stepping
end
%movie
for k=1:size(y)
	posX=y(k,1:numberOfPoints); % x position matrix
	posY=y(k,numberOfPoints+1:2*numberOfPoints); % y position matrix
	plot(posX,posY,'b.'); %plot instantaneous position
    axis([0,L,0,L]);
    axis square;
    grid on;
	pause(.005);
end