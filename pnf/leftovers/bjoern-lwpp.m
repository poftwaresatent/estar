function [totalprob] = LWPP(r, g, d, vr, vm)

%
%   Line world path planning
%
%   r - robot position
%   g - goal position
%   d - column vector of dynamic object's position
%   vr - robot mean velocity
%   vm - dynamic obstacle max velocity
%
%   P - co-occurence probability
%
%   30/03/03, btj, EPFL-ASL
%

s = 0.01;       % grid size

% compute meeting time and place
t1 = max(r-d) / (vr - vm);  s1 = r - vr*t1;
t2 = max(r-d) / (vm - vr);  s2 = r + vr*t2;
s1 = -100;  s2 = 100;
disp([t1 s1; t2 s2]);


% define plot range
minP = 1.5 * min([r;g;d;s1;s2]);
maxP = 1.5 * max([r;g;d;s1;s2]);
pos = [minP:s:maxP] + s/2;


figure(1);  clf;    hold on;    colormap(copper);
%plot([r r], [0 1.0], 'b-');     
plot(r, 0.0, 'b*');
%text(r+0.05, 1.05, 'robot');


% get parameters
dRob = abs(pos - r);    
t = dRob ./ vr;
N = t / (s / vr) + 0.5;
prob = zeros(size(d,1), size(pos,2));
totalprob = zeros(size(pos));
for i=1:size(d,1),
    dObj = pos - d(i);
    v1 = (dObj - s/2) ./ t;
    v2 = (dObj + s/2) ./ t;
    
    pleft = (v1 + vm) / vm + (N-1) ./ N .* (v1 .* v1 - vm .* vm) / (2*vm * vm);
    pbothleft = (v2 - v1) / vm + (N-1) ./ N .* (v2 .* v2 - v1 .* v1) / (2*vm * vm);
    pmiddle = (v2 - v1) / vm - (N-1) ./ N .* (v2 .* v2 + v1 .* v1) / (2*vm * vm);
    pbothright = (v2 - v1) / vm - (N-1) ./ N .* (v2 .* v2 - v1 .* v1) / (2*vm * vm);
    pright = (vm - v2) / vm - (N-1) ./ N .* (vm .* vm - v2 .* v2) / (2*vm * vm);
    
    left = (v1 < -vm) .* (-vm < v2) .* (v2 < 0);
    bothleft = (-vm <= v1) .* (v1 < 0) .* (-vm <= v2) .* (v2 < 0);
    middle = (-vm <= v1) .* (v1 < 0) .* (0 <= v2) .* (v2 < vm);
    bothright = (0 <= v1) .* (v1 < vm) .* (0 <= v2) .* (v2 < vm);
    right = (0 <= v1) .* (v1 < vm) .* (vm <= v2);
    
    % only for debugging
    if min(left .* pleft) < 0,
        test = 1;
    end;
    if min(bothleft .* pbothleft) < 0,
        test = 1;
    end;
    if min(middle .* pmiddle) < 0,
        test = 1;
    end;
    if min(bothright .* pbothright) < 0,
        test = 1;
    end;
    if min(right .* pright) < 0,
        test = 1;
    end;
    
    prob = left .* pleft + bothleft .* pbothleft + middle .* pmiddle + bothright .* pbothright + right .* pright;
    prob = prob;
    
    
    if vm==0,
        prob = zeros(size(prob)) + (dObj >= -s) .* (dObj <s);
    end;
    totalprob = totalprob + (1-totalprob).*prob;
    %plot([d(i) d(i)], [0 1.0], 'r-');   
    plot(d(i), 0.0, 'ro');
    %text(d(i)+0.05, 1.05, sprintf('%d', i));
end;



% plot the probability function
plot([minP maxP], [0 0], '-k');
plot(pos, totalprob, '-k');

axis([minP maxP [-0.1 1.1]*max(totalprob)]);