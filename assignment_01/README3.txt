# README 2 CG
*Gruppe: R. Stern, A.Helmy, B.Helmy*

Started with editing function compute_normals.
The goal was to find a solution where we compute all vertex normal in a efficient way.
Instead of finding all incident triangles for one vertex we iterated over all triangles.
For every vertex in the triangle we added the weighted normal (depending on angle).
In the end we need to normalize the vertex normal.

function intersect_bounding_box
we had some issues finding a efficient way to intersect our ray with our box.
There are 6 planes in a box but for every ray we need to check only 3
depending on the ray origin.
in addition we needed to check if the intersection is in front of the viewer.


function intersect_triangles

First of all discard all obvious ray that don't intersect with the triangle
We had some troubles finding the right formula to compute the intersection point.
When we found the intersection point and made sure that it is in front of the viewer
we differed between the two draw modes Phong and flat.
Phong: we already have computed the weighted vertex normals in "compute_normals()"
Therefore we only needed to multiply avery vertex normal with the coefficient a,b, and 1-a-b
and take the sum of the result of every normal.
Flat: the intersection normal is the triangle normal.
