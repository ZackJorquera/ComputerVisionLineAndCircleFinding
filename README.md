# Computer Vision; edges and lines
By: Zackary T. Jorquera,
May 5, 2017


# Hough
The hough space uses the polar representation of a line to convert points into a line as a stepping stone. The hough space is not really its own space it is just the method used for finding possible lines for point in in image. Although i will reference images and ideas portraying the hough space as its own space just to help understand what it does. IMG-9 shows how a single point to transferred into hough space, it turns the point into a sinusoid (a sine wave). This happens because of how the polar representation of a line represents the all of the possible lines that intersect a point. In order for every possible line to be made off of the point the algorithm goes over every possible theta value between 0 and 180 not including 180 (the 180 degrees direction is already covered by 0 degrees). An finds the only possible rho value that makes the line intersect the point (IMG-10). So for every point on the hough space in IMG-9 represents a possible line that could be drawn on the image intersecting the point. This is good and all but how can the program detect the places with most intersection without the use of slow complex math that reiterates for every possible combination of lines. This is bypassed by having the hough space be made up of ‘bins’ to collect ‘votes’ from each edge point[1]. This process is called a hough line accumulator. The bins are defined by creating a 2D array with sizes of 180 divided by the theta bin size. This means that when the point in the image space is being turned into the sinusoid it only goes through every possible theta value for every theta bin size. If the theta bin size is 0.5 degrees then the theta value that the program iterates through will be (0, 0.5, 1, 1.5, 2 …, 179, 179.5). There are also a rho bins that includes values from the diagonal length of the image times -1 to the diagonal length of the image([(-diagonal length) (diagonal length)]) taking in account the rho bin sizes. From here, using one of the theta values a possible line for a pixel can be made by calculating the value of rho with: ```rho = (x * cos(theta)) - (y * sin(theta))```. This equation can be derived from trig identities/functions. Then the bin at the location of (rho, theta) is given a ‘vote’. The value of the (rho, theta) position is one value greater than it was originally (1 goes to 2 56 goes to 57 and so on). When this is done for every theta in the range and for every edge pixel it is then mapped to the hough space like IMG-11. This image is in the form of a heat map where white pixels have the most ‘votes’, red has lower amounts of votes and black has none. This graph is arranged so that the y axis is rho and that theta is the x axis. 

The accumulator generates a 2D array with ‘votes’ in each ‘bin’. The next step is to count the amount of votes that each bin has. As a reminder each bin represents a line that can be graphed in image space and thus when finding which bins have the most votes (peaks), this is really like finding which line fits best with the most amount of edges. Although it works a similar way it is not how the line of best fit is calculated but rather a way to identify where possible lines are. This is done by first filtering out every ‘bin’ that does not have a certain ‘vote’ threshold of points. These bins with greater amounts of votes than the threshold are called peaks. When graphing the lines from the peaks, there usually are a maximum number of points that are wanted. This is why the list of peaks is then sorted so that the best possible lines are graphed on the image. In IMG-11 I have graphed a box around each peak that will be graphed as a line later. 

The last step is to draw the lines from the peaks that were outputted by the previous algorithm. This can be accomplished by the matLab function ‘plot()’ which takes in the inputs of the range that the line spans in the x direction and y direction. Because the graph of a line is traditionally written y = mx+b, the range for x is set to the image’s width (1, size(img,2)). Everything else is derived from the equation: rho = (x * cos(theta)) - (y * sin(theta)) so: ```m = cot(theta) and b = (-1 * csc(theta) * rho)```. Because x is a range, when y is set to mx + b then it also becomes a range making a line on the image (IMG-12 where the green lines have been graphed on).

This all works fine and all except when the bin size is not one. Because the bins are part of an array and when getting a stored value at a position the values for the position can only be integer values. Because of this, when storing the values the theta and rho bin values must be stretched to start at 1 and have a separation of 1. For rho this can be accomplished with: ```rho_m = int32(rhoFound/rhoBinSize+ rhoBins/2)``` where int32 converts the value to a 32 bit integer and ```rhoBins = 2*(int32(sqrt((size(img,2) - 1)^2 + (size(img,1) - 1)^2)/rhoStep)) + 1``` where size(img,1) is the height of the image and size(img,2) is the width. For theta it is ```theta_m = int32(theta/thetaBinSize) + 1```. I’m using the ‘_m’ to show that it was modified to int form to fit in the array. The + 1 in this and for the rhoBins is used to turn the 0 value into a 1 because in matlab the arrays position start at 1. Because the value have been change when the line is graphed the values have to be reverted back to the original values. To do this where the values have been modified the program has to keep track of that and this can be easily done by having an array with the same size as all of the possible theta values contaning all of the possible theta values for the thetaBinsSize. Meaning if the thetaBinSize is 0.5 then the array would be (0, 0.5, 1, 1.5, 2 …, 179, 179.5) in positions (1,2,3,4,5 …, 358, 359) respectively. The rho version works the same way. These two things are stored as two output variables, T and R. To use them is is a simple as T(position) and R(position) which converts from the modified values to the real values.

Now that we are done with lines we can move to circles and other shapes. In some ways finding a circle is easier because when finding a circle each point on the circle is the radius away from the center. Thus, no geometric math is needed and the instead of finding every possible line that a point can make you only need to find every point radius away from the circle. This is done with: ```[a b] = [int32(x - (radius * cos(theta)))    int32(y + (radius * sind(theta)))]```  where ‘a’ and ‘b’ are the x and y positions of the point radius away from the edge pixel. Note that bin sizes can be changed, although because a and b represent individual pixels on the image it is unadvised to do so as it skips over pixels. Finding peaks uses the same method of finding the most voted for bins on the hough space, except instead of sinusoids intersecting, the circle for every edge point intersect (IMG-13). When drawing the circle on the image it takes the peaks and draws a circle from its radius around the point (a,b) (IMG-14).

![alt text](https://github.com/ZackJorquera/ComputerVisionLineAndCircleFinding/blob/master/ImagesForReadme/1.jpg "Electromagnetic Spectrum")
![alt text](https://github.com/ZackJorquera/ComputerVisionLineAndCircleFinding/blob/master/ImagesForReadme/2.jpg "Electromagnetic Spectrum")
![alt text](https://github.com/ZackJorquera/ComputerVisionLineAndCircleFinding/blob/master/ImagesForReadme/3.jpg "Electromagnetic Spectrum")
![alt text](https://github.com/ZackJorquera/ComputerVisionLineAndCircleFinding/blob/master/ImagesForReadme/4.jpg "Electromagnetic Spectrum")
![alt text](https://github.com/ZackJorquera/ComputerVisionLineAndCircleFinding/blob/master/ImagesForReadme/5.jpg "Electromagnetic Spectrum")
![alt text](https://github.com/ZackJorquera/ComputerVisionLineAndCircleFinding/blob/master/ImagesForReadme/6.jpg "Electromagnetic Spectrum")


# Acknowledgement: 
This was created for my junior year high school LA research project.


# Sources: 
[1] Bobick, Aaron, Irfan Essa, and Arpan Chakraborty. "Introduction to Computer Vision."Udacity. Georgia Institute of Technology, n.d. Web. 05 May 2017. <https://www.udacity.com/course/introduction-to-computer-vision--ud810>.

[2] "Houghlines." Hough Transform - MATLAB Hough. MathWorks, n.d. Web. 05 May 2017. <http://www.mathworks.com/help/images/ref/hough.html>.
