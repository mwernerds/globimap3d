# -*- coding: utf-8 -*-
from plyfile import PlyData, PlyElement
import numpy as np;
import vispy.scene
from vispy.scene import visuals
# our teapot
from teapot import get_scaled_teapot
from helena import globimap3d
from tqdm import tqdm
import sys
#
# Make a canvas and add simple view
#
canvas = vispy.scene.SceneCanvas(keys='interactive', show=True)
view = canvas.central_widget.add_view()

scale = 100

vertices = get_scaled_teapot(scale).astype(np.uint32)

# encode as globimap3d
m = globimap3d()
m.configure(15,19) # 15 hash functions, but 2^19 bits
for v in vertices:
    m.put(v[0],v[1],v[2])
print(m.summary())

# decoding all possible locations and comparing the number of original points
# with the (higher) number of points in the data structure
from itertools import product
vertices2 = np.array([a for a in product(range(scale),range(scale),range(scale)) if m.get(a[0],a[1],a[2])])
print(vertices2)
print("Original %d: Probabilistic: %d" % (len(vertices),len(vertices2)))


# create scatter object and fill in the data
scatter = visuals.Markers()
scatter.set_data(vertices2, edge_color=None, face_color=(1, 1, 1, .5), size=5)

view.add(scatter)

view.camera = 'arcball'  # or try 'turntable'

# add a colored 3D axis for orientation
axis = visuals.XYZAxis(parent=view.scene)

if __name__ == '__main__':
    if sys.flags.interactive != 1:
        vispy.app.run()
        
