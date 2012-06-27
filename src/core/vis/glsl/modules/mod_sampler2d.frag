
uniform vec2 screenDim_;
uniform vec2 screenDimRCP_;

// definitions for textures of type GL_TEXTURE_2D
#if defined(VRN_TEXTURE_2D)

    #define SAMPLER2D_TYPE sampler2D

    // texture lookup function for 2D textures
    // texture coordinates have to be passed as fragment coordinates!
    vec4 textureLookup2D(in sampler2D texture, in vec2 texCoords) {
        vec2 texCoordsNormalized = texCoords * screenDimRCP_;
        return texture2D(texture, texCoordsNormalized);
    }
    
// definitions for textures of type GL_TEXTURE_RECTANGLE_ARB
#elif defined(VRN_TEXTURE_RECTANGLE)

    #extension GL_ARB_texture_rectangle : enable    

    #define SAMPLER2D_TYPE sampler2DRect

    // texture lookup function for 2D textures
    // texture coordinates have to be passed as fragment coordinates!
    vec4 textureLookup2D(in sampler2DRect texture, in vec2 texCoords) {
        return texture2DRect(texture, texCoords);
    }

#endif
