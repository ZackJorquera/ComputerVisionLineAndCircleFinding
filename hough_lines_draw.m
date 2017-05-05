function hough_lines_draw(img, peaks, rho, theta)
    % Draw lines found in an image using Hough transform.
    %
    % img: Image on top of which to draw lines
    % //outfile: Output image filename to save plot as
    % peaks: Qx2 matrix containing row, column indices of the Q peaks found in accumulator
    % rho: Vector of rho values, in pixels
    % theta: Vector of theta values, in degrees

    % TODO: Your code here
    %imshow(img);
    %hold on;
    
    for i = 1:size(peaks,1)
        if(theta(peaks(i,2)) == 90)
            xRange = [1 size(img,2)];
            yRange = [-rho(peaks(i,1)) -rho(peaks(i,1))];
            line(xRange,yRange,'Color','green', 'LineWidth', 2);
        elseif(theta(peaks(i,2)) == 0)
            yRange = [1 size(img,1)];
            xRange = [rho(peaks(i,1)) rho(peaks(i,1))];
            line(xRange,yRange,'Color','green', 'LineWidth', 2);
        else
            b = (-1 * cscd(theta(peaks(i,2))) * rho(peaks(i,1))); 
            m = cotd(theta(peaks(i,2)));
            x = linspace(1,size(img,2)); % Adapt n for resolution of graph
            y = m*x + b;
            plot(x,y,'Color','green', 'LineWidth', 2)
        end
    end
end
