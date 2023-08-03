from plyfile import PlyData, PlyElement
import numpy as np;



def get_scaled_teapot(scale=10):
    plydata = PlyData.read('teapot_body.ply')
    assert(plydata.elements[0].name == "vertex"),"First element is not vertex, reshape the reader [or the file]"
    vertices = plydata.elements[0].data
    vertices = vertices.view(np.float32).reshape(-1,8)[:,:3]
    # scale to unit cube
    vertices = vertices - np.min(vertices,axis=0)
    vertices = vertices / (np.max(vertices,axis=0) / scale)
    return (vertices)


if __name__=="__main__":
    print(get_scaled_teapot())
