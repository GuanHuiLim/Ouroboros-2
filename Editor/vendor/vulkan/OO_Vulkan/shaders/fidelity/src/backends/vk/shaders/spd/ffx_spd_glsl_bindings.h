// SPD pass
// SRV  0 : SPD_InputDownsampleSrc          : r_input_downsample_src
// UAV  0 : SPD_InternalGlobalAtomic        : rw_internal_global_atomic
// UAV  1 : SPD_InputDownsampleSrcMidMip    : rw_input_downsample_src_mid_mip
// UAV  2 : SPD_InputDownsampleSrcMips      : rw_input_downsample_src_mips
// CB   0 : cbSPD


#define FFX_SPD_BIND_SRV_INPUT_DOWNSAMPLE_SRC                  0

#define FFX_SPD_BIND_UAV_INTERNAL_GLOBAL_ATOMIC             2000
#define FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MID_MIPMAP    2001
#define FFX_SPD_BIND_UAV_INPUT_DOWNSAMPLE_SRC_MIPS          2002

#define FFX_SPD_BIND_CB_SPD                                 3000