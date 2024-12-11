# nighthawks

**Team Name:**
gas

**Team Members:**
Alexander Jiang, Emily Wang, Khalil Desai

**Overview:**
Welcome to gas's final project! Thanks to some artistic inspiration from Alex, we chose to try and 
recreate the classic Edward Hopper painting "Nighthawks" using some of the realtime 3D graphics tools
we've learned from this semester and some new techniques. Originally an oil-painting created in 1942,
"Nighthawks" is one of the most well-known pieces of American art and is emblematic of the mid-20th century
era.

Our recreation guides the camera down from above a procedurally generate array of buildings along a predetermined path.
The camera is drawn into position as we peer into the diner at the center --- one which has some new and strange visitors!

We hope you enjoy our project!

**Design:**

*Painterly Shader:*
In order to give the diner at the center of our scene the effect of being "painted," we implemented a non-photorealistic
shader designed to make scenes look especially stylized. In particular, we wrote a post-processing effect called the "Kuwahara"
filter using a texture shader. This filter calculates the value of each pixel by 1) centering a box at the pixel 2) dividing the box
into 4 sectors 3) calculating the average color and variance for each sector and 4) choosing the average color with the lowest variance.

The shader looks slightly better when used on more dynamic scenes --- we include an example of a scene where it looks particularly good
alongside our main scene in our video. Nevertheless, it functions well in adding an artistic flourish to our scene.

*Bezier Curves:*

*Procedural Generation of Buildings:*


**Mentor TA:**
Sophie Zhang

**Collaborators:**
None outside of our team and Sophie!

**Potential Further Steps:**
N
