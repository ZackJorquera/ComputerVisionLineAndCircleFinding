function hough_circles_draw(peaks)
%HOUGH_CIRCLES_DRAW Summary of this function goes here
%   Detailed explanation goes here
    %imshow(BW);
    %hold on;

    for i = 1:size(peaks)
        th = 0:pi/50:2*pi;
        radius = peaks(i,3);
        xunit = radius * cos(th) + peaks(i,2) - radius;
        yunit = peaks(i,1) - (radius * sin(th)) - radius;
        plot(xunit, yunit,'Color','green', 'LineWidth', 2);
    end

    %hold off;
        
end

