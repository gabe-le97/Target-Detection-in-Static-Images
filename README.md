# Target-Detection-in-Static-Images
Using background subtraction to detect "objects" in static images, display the "objects" on the frame image,
& save an output image file of the difference.

![targets](https://raw.githubusercontent.com/gabe-le97/Target-Detection-in-Static-Images/master/img/target.png)

We read two TGA files: a background image & a frame image. Then do background subtraction and find the
different objects between the two images. Finally, display the objects on the frame image and then save
the objects as its own TGA file.

***
__Background subtraction__: Assuming we have a reference "background" image of a scene, we can compute the 
difference between a new image by checking the pixel difference between the two images. 
1. Convert both images to their gray-level equivalent by averaging every rgb pixel value.
2. Compute the absolute value between the gray-level pixels.
3. Using a blob detection algorithm, find connected pixels in an image.

Multithreaded for background subtraction. Each thread computes a row of pixels for the image and when finished,
the blob detection code is run and the output is given.
