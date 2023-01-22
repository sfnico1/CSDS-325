

% wget code
figure(1)
A = readmatrix('wget.txt');
[row,~] = size(A);
hold on

index = 1;
counter = 0;
done = 1;
while done
    for i = 2:11
        if i ~= 2
            data(index,i-1) = A(index,i) - A(index,i-1);
        else 
            data(index,i-1) = A(index,i);
        end
    end
    if index >= row
        done = 0;
        break
    end
    index = index + 1;
end

color = ["#EDB120", "r", "g", "b", "c", "m", "#A2142F", "k", "#0072BD", "#D95319", "#AE8312","#C91A51","#387ADE"];
website = ["", "case.edu", "stanford.edu", "gov.za","japan.jp", "unimelb.edu.au", "nyse.com", "globo.com", "koora.com ", ...
    "futbollibre.net", "infobae.com"];

hold on
for i = 2:11
    plot(A(:,1)/3600, A(:,i)*1000, 'Color', color(i), 'LineWidth', 1)
end
legend("case.edu - Cleveland, OH", "stanford.edu - Stanford, CA", "gov.za - East London, South Africa","japan.jp - Tokyo, Japan", ...
    "unimelb.edu.au - Melbourne, Australia", "nyse.com - New York City, NY", "globo.com - Rio de Janeiro, Brazil", "koora.com - Paris, France", ...
    "futbollibre.net - Helsinki, Finland", "infobae.com - Mumbai, India")
title("wget Times to websites")
ylabel("Milliseconds (ms)")
xlabel("t+ start time of 11PM EST (hours)")



%%

% curl code
figure(2)
hold on
A = readmatrix('curl.txt');
[row,~] = size(A);

index = 1;
counter = 0;
done = 1;
while done
    for i = 1:13
        counter = counter + 1;
        DNSdata(index,i) = A(counter,2);
        TCPdata(index,i) = A(counter,3);
        BYTEdata(index,i) = A(counter,4);
        TTdata(index,i) = A(counter,5);
    end
    if counter >= row
        done = 0;
        break
    end
    index = index + 1;
end

color = ["#EDB120", "r", "g", "b", "c", "m", "#A2142F", "k", "#0072BD", "#D95319", "#AE8312","#C91A51","#387ADE"];
website = ["case.edu", "stanford.edu", "gov.za","japan.jp", "unimelb.edu.au", "nyse.com", "globo.com", "koora.com ", ...
    "futbollibre.net", "infobae.com", "canvas.case.edu", "sis.case.edu", "mail.google.com"];

for i = 1:13
    subplot(4,4,i)
    plot((1:480)/6, DNSdata(:,i)*1000, 'Color', color(i), 'LineWidth', 2)
    title("DNS Lookup Times to " + website(i))
    ylabel("Milliseconds (ms)")
    xlabel("t+ start time of 11PM EST (hours)")
end


figure(3)
for i = 1:13
    subplot(4,4,i)
    plot((1:480)/6, TCPdata(:,i)*1000, 'Color', color(i), 'LineWidth', 2)
    title("TCP Connection Times to " + website(i))
    ylabel("Milliseconds (ms)")
    xlabel("t+ start time of 11PM EST (hours)")
end

figure(4)
for i = 1:13
    subplot(4,4,i)
    plot((1:480)/6, BYTEdata(:,i)*1000, 'Color', color(i), 'LineWidth', 2)
    title("First Byte Sent Times to " + website(i))
    ylabel("Milliseconds (ms)")
    xlabel("t+ start time of 11PM EST (hours)")
end

figure(5)
for i = 1:13
    subplot(4,4,i)
    plot((1:480)/6, TTdata(:,i)*1000, 'Color', color(i), 'LineWidth', 2)
    title("Total Times to " + website(i))
    ylabel("Milliseconds (ms)")
    xlabel("t+ start time of 11PM EST (hours)")
end


