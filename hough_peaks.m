function peaks = hough_peaks(H, varargin)
    % Find peaks in a Hough accumulator array.
    %
    % Threshold (optional): Threshold at which values of H are considered to be peaks
    % NHoodSize (optional): Size of the suppression neighborhood, [M N]

    %% Parse input arguments
    p = inputParser;
    addOptional(p, 'numpeaks', 1, @isnumeric);
    addParameter(p, 'Threshold', 0.5 * max(H(:)));
    addParameter(p, 'NHoodSize', floor(size(H) / 100.0) * 2 + 1);  % odd values >= size(H)/50
    parse(p, varargin{:});

    numpeaks = p.Results.numpeaks;
    threshold = p.Results.Threshold;
    nHoodSize = p.Results.NHoodSize;
    
    if(size(H,3)>1)
        H_m = H;
    else
        H_m(:,:,1) = H(:,:);
    end
    
    
    Qon = 1;
    nHoodSize = nHoodSize/2;
    % TODO: Your code here
    for y = 1:size(H_m,1)
        for x = 1:size(H_m,2)
            for z = 1:size(H_m,3)
                if(H_m(y,x,z) > threshold)
                    allPeaks(Qon,3) = z;
                    allPeaks(Qon,2) = x;
                    allPeaks(Qon,1) = y;
                    Qon = Qon + 1;
                end
            end
        end
    end
    if(size(allPeaks, 1) > numpeaks)
        for i = 0:numpeaks-1
            greatestPeak = [1,1,1];
            for j = 0:size(allPeaks, 1)-1
                if(H_m(allPeaks(j+1,1), allPeaks(j+1,2), allPeaks(j+1,3)) > H_m(greatestPeak(1), greatestPeak(2), greatestPeak(3)))
                    greatestPeak = [allPeaks(j+1,1) allPeaks(j+1,2), allPeaks(j+1,3)];
                end
            end
            peaks(i+1, 2) = greatestPeak(2);
            peaks(i+1, 1) = greatestPeak(1);
            peaks(i+1, 3) = greatestPeak(3);
            if(size(H_m,3) > 1)
                H_m(greatestPeak(1), greatestPeak(2), greatestPeak(3)) = 0;
            else
                H_m(greatestPeak(1), greatestPeak(2), 1) = 0;
            end
            
            for nHoodX = int32(-nHoodSize:nHoodSize)
                for nHoodY = int32(-nHoodSize:nHoodSize)
                    for nHoodZ = int32(-nHoodSize:nHoodSize)
                        if((greatestPeak(2) + nHoodY) > 0 && greatestPeak(2) + nHoodY <= size(H_m, 2) && (greatestPeak(1) + nHoodX) > 0 && greatestPeak(1) + nHoodX <= size(H_m, 1))
                            if(size(H_m,3) > 1)
                                if((greatestPeak(3) + nHoodZ) > 0 && greatestPeak(3) + nHoodZ <= size(H_m, 3))
                                    if(nHoodX == 0 && nHoodY == 1 && nHoodZ == 3)
                                        nHoodX = nHoodX;
                                    end
                                    H_m(greatestPeak(1) + nHoodX, greatestPeak(2) + nHoodY, greatestPeak(3) + nHoodZ) = 0;
                                end
                            else
                                H_m(greatestPeak(1) + nHoodX, greatestPeak(2) + nHoodY) = 0;
                            end
                        end
                    end
                end
            end
        end
    else
        peaks = allPeaks;
    end
    
    
end
