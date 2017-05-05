function [H, theta, rho] = hough_lines_acc(BW, varargin)
    % Compute Hough accumulator array for finding lines.
    %
    % BW: Binary (black and white) image containing edge pixels
    % RhoResolution (optional): Difference between successive rho values, in pixels
    % Theta (optional): Vector of theta values to use, in degrees
    %
    % Please see the Matlab documentation for hough():
    % http://www.mathworks.com/help/images/ref/hough.html
    % Your code should imitate the Matlab implementation.
    %
    % Pay close attention to the coordinate system specified in the assignment.
    % Note: Rows of H should correspond to values of rho, columns those of theta.

    %% Parse input arguments
    p = inputParser();
    addParameter(p, 'RhoResolution', 1);
    addParameter(p, 'Theta', linspace(-90, 89, 180));
    parse(p, varargin{:});

    rhoStep = p.Results.RhoResolution;
    thetaStep = p.Results.Theta;
    
    rhoBins = 2*(ceil(sqrt((size(BW,2) - 1)^2 + (size(BW,1) - 1)^2)/rhoStep)) + 1;
    thetaBins = int32(180/thetaStep) + 1;
    
    
    H = zeros([rhoBins thetaBins 1]);
    theta = double(([thetaBins]));
    rho = ([rhoBins]);

    %% TODO: Your code here
    for i = 1:rhoBins
        rho(i) =  (double(i) - rhoBins/2) * rhoStep;
    end
    for i = 1:thetaBins-1
        theta(i) =  (double(i)*thetaStep) - thetaStep;
    end
    
    for y = 1:size(BW,1)
        for x = 1:size(BW,2)
            if(BW(y,x) == 1)
                for tryTheta = 0:thetaStep:(180 - thetaStep)
                   rhoFound = (x * cosd(tryTheta)) - (y * sind(tryTheta));
                   rhoFound_m = int32(rhoFound/rhoStep + rhoBins/2);
                   tryTheta_m = int32(tryTheta/thetaStep) + 1;
                   H(rhoFound_m, tryTheta_m, 1) = H(rhoFound_m, tryTheta_m, 1) + 1;
                end
            end
        end
    end
end
