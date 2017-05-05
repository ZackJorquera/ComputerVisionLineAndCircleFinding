function H = hough_circles_acc(BW, radius)
    % Compute Hough accumulator array for finding circles.
    %
    % BW: Binary (black and white) image containing edge pixels
    % radius: Radius of circles to look for, in pixels

    % TODO: Your code here
    H = zeros([(size(BW,1) + 2* max(radius)) (size(BW,2) + 2* max(radius)) max(radius)]);%makes the 3d array
    
    for y = 1:size(BW,1)
        for x = 1:size(BW,2)
            if(BW(y,x) == 1)
                for tryTheta = 0:360-1
                    for tryRadius = radius
                        a = int32(x - (tryRadius * cosd(tryTheta)));
                        b = int32(y + (tryRadius * sind(tryTheta)));
                        H(b+max(radius),a+max(radius), tryRadius) = H(b+max(radius),a+max(radius), tryRadius) + 1;
                    end
                end
            end
        end
    end
end
