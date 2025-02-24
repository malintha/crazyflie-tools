
t = crazyflie_state_estimate(:,14);

state_estimates = crazyflie_state_estimate(:,2:13);
% estimates = zeros(size(state_estimates));
% for i=1:size(estimates,1)
%   R = rpy2rotmat(state_estimates(i,4:6));
%   rpy = angularvel2rpydot(state_estimates(i,4:6),R*state_estimates(i,10:12)');
%   estimates(i,:) = [state_estimates(i,1:9),rpy'];
% end
% estimates = PPTrajectory(foh(crazyflie_state_estimate(:,14),estimates'));
estimates = PPTrajectory(foh(crazyflie_state_estimate(:,14),state_estimates'));
estimates_samples = estimates.eval(t);

vicon_raw = crazyflie2_squ_ext(:,2:7);
vicon = zeros(size(vicon_raw,1),12);
for i=1:size(vicon,1)
  vicon(i,1:3) = vicon_raw(i,1:3);
  % actually need to rotate w.r.t the rpy of the imu because the yaw is
  % diffferent...
  xyzrpy = estimates.eval(crazyflie2_squ_ext(i,8));
  vicon(i,4:6) = quat2rpy(angle2quat(xyzrpy(4),xyzrpy(5),xyzrpy(6),'XYZ'));
end
for i=2:size(vicon,1)
  vxyz = 120*(vicon(i,1:3)-vicon(i-1,1:3));
  vrpy = 120*(vicon(i,4:6)-vicon(i-1,4:6));
  if (vxyz==0)
    vicon(i,7:9) = vicon(i-1,7:9);
  else
    vicon(i,7:9) = vxyz;
  end
  if (vrpy==0)
    vicon(i,10:12) = vicon(i-1,10:12);
  else
    vicon(i,10:12) = vrpy;
  end
end
vicon = PPTrajectory(foh(crazyflie2_squ_ext(:,8),vicon'));
vicon_samples = vicon.eval(t);

t0 = 5;
tf = 10;

figure(1)
hold on
plot(t,vicon_samples(10,:),'bx-');
plot(t,estimates_samples(10,:),'rx-');
ylim([-10 10]);
xlim([t0 tf])
figure(2)
hold on
plot(t,vicon_samples(11,:),'bx-');
plot(t,estimates_samples(11,:),'rx-');
ylim([-10 10]);
xlim([t0 tf])
figure(3)
hold on
plot(t,vicon_samples(12,:),'bx-');
plot(t,estimates_samples(12,:),'rx-');
ylim([-10 10]);
xlim([t0 tf])