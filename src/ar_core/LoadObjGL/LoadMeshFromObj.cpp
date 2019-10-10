#include "LoadMeshFromObj.h"

#include "GLInstanceGraphicsShape.h"
#include <BulletCollision/CollisionShapes/btConvexHullShape.h>
#include <iostream>
#include <sys/stat.h>
//#include "src/ar_core/LoadObjGL/VHACDGen.h"
#include "VHACDGen.h"
#include "Bullet3Common/b3HashMap.h"
#include "Wavefront2GLInstanceGraphicsShape.h"

struct CachedObjResult
{
    std::string m_msg;
    std::vector<tinyobj::shape_t> m_shapes;
};

static b3HashMap<b3HashString, CachedObjResult> gCachedObjResults;
static int gEnableFileCaching = 1;

void b3EnableFileCaching(int enable)
{
    gEnableFileCaching  = enable;
    if (enable==0)
    {
        gCachedObjResults.clear();
    }
}


inline bool FileExists (const std::string& name) {
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}


std::string LoadFromCachedOrFromObj(
        std::vector<tinyobj::shape_t>& shapes,   // [output]
        const char* filename,
        const char* mtl_basepath)
{
    CachedObjResult* resultPtr = gCachedObjResults[filename];
    if (resultPtr)
    {
        const CachedObjResult& result = *resultPtr;
        shapes = result.m_shapes;
        return result.m_msg;
    }

    std::string err = tinyobj::LoadObj(shapes, filename, mtl_basepath);
    CachedObjResult result;
    result.m_msg = err;
    result.m_shapes = shapes;
    if (gEnableFileCaching)
    {
        gCachedObjResults.insert(filename,result);
    }
    return err;
}


GLInstanceGraphicsShape* LoadMeshFromObj(const std::string relativeFileName)
{
    B3_PROFILE("LoadMeshFromObj");
    std::vector<tinyobj::shape_t> shapes;
    {
        B3_PROFILE("tinyobj::LoadObj2");
        std::string  err  = LoadFromCachedOrFromObj(shapes, relativeFileName
                .c_str(), "");
    }

    {
        B3_PROFILE("btgCreateGraphicsShapeFromWavefrontObj");
        GLInstanceGraphicsShape* gfxShape = btgCreateGraphicsShapeFromWavefrontObj(shapes);
        return gfxShape;
    }
}



btCompoundShape *LoadCompoundMeshFromObj(const std::string relativeFileName,
                                         const float scaling_factor)

{
    // If there is no file with the same name ending with _hacd we need to
    // decompose the mesh
    if(!FileExists(AddHACDToName(relativeFileName)))
        if(DecomposeObj(relativeFileName) < 0)
            return 0;
    std::string file_name = AddHACDToName(relativeFileName);

    B3_PROFILE("LoadMeshFromObj");
    std::vector<tinyobj::shape_t> shapes;
    {
        B3_PROFILE("tinyobj::LoadObj2");
        std::string  err  = LoadFromCachedOrFromObj(shapes, file_name
                .c_str(), "");
    }


    btCompoundShape* compound = new btCompoundShape();

    for (int i = 0; i < shapes.size(); ++i) {
        std::vector<tinyobj::shape_t> shape = {shapes[i]};

        GLInstanceGraphicsShape *gfxShape = btgCreateGraphicsShapeFromWavefrontObj(
                shape);

        // create convex hull
        const GLInstanceVertex &v = gfxShape->m_vertices->at(0);
        btConvexHullShape *btCHshape = new btConvexHullShape(
                (const btScalar *) (&(v.xyzw[0])),
                gfxShape->m_numvertices,
                sizeof(GLInstanceVertex));
        btCHshape->setLocalScaling(btVector3(scaling_factor, scaling_factor, scaling_factor));
        btCHshape->initializePolyhedralFeatures();
        btCHshape->optimizeConvexHull();
        btCHshape->setMargin(0.0);

        //calculate the centroid
        btVector3 centroid(0.0,0.0,0.0);
        for (int j = 0; j <gfxShape->m_numvertices; ++j) {
            centroid += btVector3(gfxShape->m_vertices->at(j).xyzw[0],
                                  gfxShape->m_vertices->at(j).xyzw[1],
                                  gfxShape->m_vertices->at(j).xyzw[2]);
        }

        centroid = centroid/(float(gfxShape->m_numvertices) );

        //float orig_dist = (float)sqrt(centroid.x()*centroid.x()  +
        //centroid.y()*centroid.y() + centroid.z()*centroid.z());
        //std::cout << i << ": centroid_dist "<< orig_dist <<std::endl;

        // add to the compound shape
        btTransform trans;
        trans.setIdentity();
        trans.setOrigin(centroid);
        compound->addChildShape(trans, btCHshape);
    }

    return compound;
}

