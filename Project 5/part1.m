

% ping code
figure(1)
hold on
A = readmatrix('ping.txt');

color = ["", "r", "g", "b", "c", "m", "#A2142F", "k", "#0072BD", "#D95319", "#EDB120"];
data = zeros([2 10]);
for i = 2:11
    plot(A(:,1)/3600, A(:,i), 'Color', color(i), 'LineWidth', 2) 
    data(1,i) = mean(A(:,i));
    data(2,i) = var(A(:,i));
end
legend("case.edu - Cleveland, OH", "stanford.edu - Stanford, CA", "gov.za - East London, South Africa","japan.jp - Tokyo, Japan", ...
    "unimelb.edu.au - Melbourne, Australia", "nyse.com - New York City, NY", "globo.com - Rio de Janeiro, Brazil", "koora.com - Paris, France", ...
    "futbollibre.net - Helsinki, Finland", "infobae.com - Mumbai, India")
title("Ping Delays to Websites")
ylabel("Milliseconds (ms)")
xlabel("t+ start time of 11PM EST (hours)")



%%
% bonus google code
figure(2)
hold on
bonus = [119.862 274.058 239.762 178.970 282.821 208.383 301.123 141.271 47.76 47.487  ...
    54.199 54.672 54.267 47.842 54.222 54.356 55.143 54.354 54.283 48.082 48.551 54.234 47.619];

plot(1:23, bonus, 'g-*','LineWidth', 2)
ylabel("Milliseconds (ms)")
xlabel("Ping Number")
title("Ping Delay to 'mail.google.com' on December 10th, 2022")

