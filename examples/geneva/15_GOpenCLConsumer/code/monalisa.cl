/*******************************************************************************************/
/**
 * A struct holding the coordinates, colors and opacity of a single triangle, which is 
 * defined via a surrounding circle
 */
typedef struct triangle_circle_struct {
  float middleX;
  float middleY;
  float radius;
  float angle1;
  float angle2;
  float angle3;
  float4 rgba_f;
  float4 dummy1; // padding
  float2 dummy2; // padding
} __attribute__ ((aligned(64))) t_circle; 

/*******************************************************************************************/
/**
 * A struct holding the coordinates, colors and opacity of a single triangle in condensed form
 */
typedef struct triangle_condensed_struct {
  float2 tr_one;
  float2 tr_two;
  float2 tr_three;
  float4 rgba_f;
  float4 dummy1; // padding
  float2 dummy2; // padding
} __attribute__ ((aligned(64))) t_cart;

/*******************************************************************************************/
/**
 * This kernel tests copying of the t_circle array to ensure that all data has arrived correctly.
 * It is meant for debugging purposes only.
 */
__kernel void monalisa_triangle_wt (
 	__global const t_circle *triangles_in
 	,  __global t_circle *triangles_out
) {
  // Retrieve our global id
  int gid = get_global_id(0);

  // Copy the data of "our" triangle to private memory
  float middleX  = triangles_in[gid].middleX;
  float middleY  = triangles_in[gid].middleY;
  float radius   = triangles_in[gid].radius;
  float angle1   = triangles_in[gid].angle1;
  float angle2   = triangles_in[gid].angle2;
  float angle3   = triangles_in[gid].angle3;
  float4 rgba_f  = triangles_in[gid].rgba_f;
  
  // Copy the data over to the output array
  triangles_out[gid].middleX = middleX;
  triangles_out[gid].middleY = middleY;
  triangles_out[gid].radius = radius;
  triangles_out[gid].angle1 = angle1;
  triangles_out[gid].angle2 = angle2;
  triangles_out[gid].angle3 = angle3;
  triangles_out[gid].rgba_f = rgba_f;
}

/*******************************************************************************************/
/**
 * This kernel calculates cartesic coordinates for the triangle edges. It then encodes the 
 * triangles so that they fit into a 128 bit wide struct, holding 6 floats for the 
 * coordinates plus 4 ushort values for the colors and opacity)
 */
__kernel void monalisa_triangle_transcode (
					  __global const t_circle *triangles_h // The triangle specifications, as provided by the host
					  , __global t_cart  *triangles_c // The condensed triangle specifications, as passed to the picture evaluation
					   ) {
  // Retrieve our global id
  int gid = get_global_id(0);

  // Copy the data of "our" triangle to private memory
  float middleX  = triangles_h[gid].middleX;
  float middleY  = triangles_h[gid].middleY;
  float radius   = triangles_h[gid].radius;
  float angle1   = triangles_h[gid].angle1;
  float angle2   = triangles_h[gid].angle2;
  float angle3   = triangles_h[gid].angle3; 
  float4 rgba_f  = triangles_h[gid].rgba_f;

  // and store them in the triangle struct
  triangles_c[gid].tr_one = (float2)(
				     mad(radius, native_cos(angle1*2.0f*M_PI_F), middleX)   // similar to middleX+radius*cos(angle1*2.0f*M_PI_F);
				     , mad(radius, native_sin(angle1*2.0f*M_PI_F), middleY) // similar to middleY+radius*sin(angle1*2.0f*M_PI_F); 
				     );
  triangles_c[gid].tr_two = (float2)(
				     mad(radius, native_cos(angle2*2.0f*M_PI_F), middleX)   // similar to middleX+radius*cos(angle2*2.0f*M_PI_F); 
				     , mad(radius, native_sin(angle2*2.0f*M_PI_F), middleY) // similar to middleY+radius*sin(angle2*2.0f*M_PI_F); 
				     );
  triangles_c[gid].tr_three = (float2)(
				     mad(radius, native_cos(angle3*2.0f*M_PI_F), middleX)   // similar to middleX+radius*cos(angle3*2.0f*M_PI_F);
				     , mad(radius, native_sin(angle3*2.0f*M_PI_F), middleY) // similar to middleY+radius*sin(angle3*2.0f*M_PI_F);
				     );

  triangles_c[gid].rgba_f = rgba_f;
}

/*******************************************************************************************/
/**
 * Checks whether a given pixel is inside a given triangle, and if so, adds the triangles
 * colors to the pixel
 */
inline void checkAndAdd(__global t_cart *t, float4 *c_pixel, float2 *pos_f) {
  float dot11 = dot(t->tr_three - t->tr_one, t->tr_three - t->tr_one);
  float dot12 = dot(t->tr_three - t->tr_one, t->tr_two - t->tr_one);
  float dot22 = dot(t->tr_two - t->tr_one, t->tr_two - t->tr_one);
  float dot1p = dot(t->tr_three - t->tr_one, *pos_f - t->tr_one);
  float dot2p = dot(t->tr_two - t->tr_one, *pos_f - t->tr_one);

  float denom = fmax(dot11*dot22 - dot12*dot12, 0.0000001f);

  float u = native_divide((dot22*dot1p - dot12*dot2p), denom);
  float v = native_divide((dot11*dot2p - dot12*dot1p), denom);

  if((u >= 0.f) && (v >= 0.f) && (u+v < 1.f)) {
    float4 alphaVec = (float4)((t->rgba_f).s3);
    *c_pixel = mix(*c_pixel, t->rgba_f, alphaVec);
  }
}

/*******************************************************************************************/
/**
 * This kernel checks for a given number of pixels whether they are contained in a set of 
 * triangles, ordered according to their alpha channel. If a pixel is contained in a triangle,
 * it adds its colors to the pixel. The kernel finally calculates the the difference between
 * these pixels and the corresponding pixels of a target image.
 */
__kernel void monalisa_candidate_creator (
					  __global t_cart *triangles_c
					  , __write_only image2d_t candidate
					  , float4 bgCol
){
  // Initialize the candidate pixel with the chosen background color
  float4 c_pixel = (float4)(bgCol.x, bgCol.y, bgCol.z, 1.f);
  float2 pos_f = (float2)((float)(get_global_id(0)+1)*XDIMINV, (float)(get_global_id(1)+1)*YDIMINV);

  for(size_t i_t=0; i_t<NTRIANGLES; i_t++) {
    checkAndAdd(triangles_c+i_t, &c_pixel, &pos_f);
  }
  
  write_imagef(candidate, (int2)(get_global_id(0), get_global_id(1)), c_pixel);
}
	
/*******************************************************************************************/
/**
 * This kernel takes a candidate and a target image and calculates the sum of the per-pixel 
 * difference between both.
 */
__kernel void monalisa_candidate_deviation (
					    __read_only image2d_t candidate
					    , __read_only image2d_t target
					    , __global float *global_results
					    ) {
  size_t pixel_id = 0;
  size_t group_id = get_group_id(0), local_id = get_local_id(0), local_size = get_local_size(0);

  float result = 0.f;
  __local float results[WORKGROUPSIZE];
  const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_ADDRESS_NONE;

  int2 pos_i; 

  float4 c_pixel, t_pixel; // The candidate and target pixels
  float4 alphaEliminator = (float4)(1.f,1.f,1.f,0.f);

  pixel_id = group_id*local_size + local_id;    
  pos_i = (int2)(pixel_id%XDIM, pixel_id/XDIM);
  
  c_pixel = read_imagef(candidate, sampler, pos_i);
  t_pixel = read_imagef(target, sampler, pos_i);
  
  c_pixel *= alphaEliminator;
  t_pixel *= alphaEliminator;

  // result += fast_distance(c_pixel, t_pixel);
  result += sqrt(dot(c_pixel-t_pixel, c_pixel-t_pixel));

  // Store the result in the local results array
  results[local_id] = result;

  // Wait for all results to be written
  barrier(CLK_LOCAL_MEM_FENCE);

  // Sum up our local results and add it to the global array
  if(local_id == 0) {
    for(size_t i_w=1; i_w<WORKGROUPSIZE; i_w++) {
      result += results[i_w];
    }
    
    global_results[group_id] = result;
  }
}

/*******************************************************************************************/
