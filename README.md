mahv (محو)
====

Criminisi's Image Inpainting Algorithm

Requirements
============

* c++
* opencv 2.4.9
* cmake >= 2.8

Running the code
================

```
cd build
cmake ..
make
./main.app
```


The GUI for selecting target region
====================================

What is the target region?
---------------------------

Target region is the part of the image that one needs to remove. The target region will get filled such that the human eye will not notice the removal.

Behavior
----------

The user must be able to select any region of the image with clicks of mouse. The region that the user selects will be a polygon resulted from clicks of the mouse.

At first the user is in __no_selection__ mode.

The user starts with a click. When the first click is done, the user is in __selection__ mode. In this mode more and more points are added to the set of points that make the polygon, or by using `u` keystroke (as undo) the last point gets removed from the polygon. Pressing `u` can result in going back to __no_selection__ mode if the set of points gets empty.

Exiting the __selection__ mode can happen in two ways: 1. When the user uses _right click_, which results in finishing the polygon and going to __done__ mode. Or When the user clicks `esc` keystroke (as cancel) that results to going back to __no_selection__ mode.

![GUI Flowchart](https://github.com/yassersouri/mahv/blob/master/extras/gui-flowchart.png)