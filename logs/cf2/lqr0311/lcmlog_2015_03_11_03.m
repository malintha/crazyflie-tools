function [d imFnames]=lcmlog_2015_03_11_03()
full_fname = 'logs/cf2/lqr0311/lcmlog_2015_03_11_03.mat';
fname = '/home/blandry/code/crazyflie-tools/logs/cf2/lqr0311/lcmlog_2015_03_11_03.mat';
if (exist(full_fname,'file'))
    filename = full_fname;
else
    filename = fname;
end
d = load(filename);
