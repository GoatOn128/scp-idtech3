# Build digest

commit: 520c53f7aa5b

- linux-x86-64-Debug: success
- linux-x86-64-Release: success
- macos-arm64-Debug: success
- macos-arm64-Release: success
- windows-x86-Debug: success
- windows-x86-Release: success

## linux-x86-64-Debug

status: success

errors: 0
warnings: 46 in 6 kinds

### errors


### failure context


### warning kinds

- [31x] "CHAR_WIDTH" redefined
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_local.h:79: warning: "CHAR_WIDTH" redefined
- [7x] -sizeof-array-argument
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_matrix.h:94:35: warning: ‘sizeof’ on array function parameter ‘src’ will return size of ‘float (*)[3]’ [-Wsizeof-array-argument]
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:807:36: warning: ‘sizeof’ on array function parameter ‘result’ will return size of ‘int*’ [-Wsizeof-array-argument]
- [2x] format ‘%d’ expects argument of type ‘int’, but argument 3 has type ‘long int’ [-Wformat=]
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2441:34: warning: format ‘%d’ expects argument of type ‘int’, but argument 3 has type ‘long int’ [-Wformat=]
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2547:34: warning: format ‘%d’ expects argument of type ‘int’, but argument 3 has type ‘long int’ [-Wformat=]
- [2x] ‘char* strncat(char*, const char*, size_t)’ specified bound 1024 equals destination size [-Wstringop
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:996:32: warning: ‘char* strncat(char*, const char*, size_t)’ specified bound 1024 equals destination size [-Wstringop-overflow=]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:2838:32: warning: ‘char* strncat(char*, const char*, size_t)’ specified bound 1024 equals destination size [-Wstringop-overflow=]
- [2x] -enum-compare
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:986:30: warning: comparison between ‘enum alphaGen_t’ and ‘enum colorGen_t’ [-Wenum-compare]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:1898:33: warning: comparison between ‘enum alphaGen_t’ and ‘enum colorGen_t’ [-Wenum-compare]
- [2x] -int-to-pointer-cast
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:116:44: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:120:16: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]

## linux-x86-64-Release

status: success

errors: 0
warnings: 51 in 9 kinds

### errors


### failure context


### warning kinds

- [31x] "CHAR_WIDTH" redefined
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_local.h:79: warning: "CHAR_WIDTH" redefined
- [7x] -sizeof-array-argument
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_matrix.h:94:35: warning: ‘sizeof’ on array function parameter ‘src’ will return size of ‘float (*)[3]’ [-Wsizeof-array-argument]
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:807:36: warning: ‘sizeof’ on array function parameter ‘result’ will return size of ‘int*’ [-Wsizeof-array-argument]
- [4x] -unused-result
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:1166:14: warning: ignoring return value of ‘size_t fread(void*, size_t, size_t, FILE*)’ declared with attribute ‘warn_unused_result’ [-Wunused-result]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:1137:14: warning: ignoring return value of ‘size_t fread(void*, size_t, size_t, FILE*)’ declared with attribute ‘warn_unused_result’ [-Wunused-result]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:204:22: warning: ignoring return value of ‘int chdir(const char*)’ declared with attribute ‘warn_unused_result’ [-Wunused-result]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:378:15: warning: ignoring return value of ‘char* getcwd(char*, size_t)’ declared with attribute ‘warn_unused_result’ [-Wunused-result]
- [2x] format ‘%d’ expects argument of type ‘int’, but argument 3 has type ‘long int’ [-Wformat=]
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2441:34: warning: format ‘%d’ expects argument of type ‘int’, but argument 3 has type ‘long int’ [-Wformat=]
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2547:34: warning: format ‘%d’ expects argument of type ‘int’, but argument 3 has type ‘long int’ [-Wformat=]
- [2x] -enum-compare
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:986:30: warning: comparison between ‘enum alphaGen_t’ and ‘enum colorGen_t’ [-Wenum-compare]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:1898:33: warning: comparison between ‘enum alphaGen_t’ and ‘enum colorGen_t’ [-Wenum-compare]
- [2x] -int-to-pointer-cast
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:116:44: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:120:16: warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]
- [1x] -aggressive-loop-optimizations
  - /home/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:1576:47: warning: iteration 2 invokes undefined behavior [-Waggressive-loop-optimizations]
- [1x] ‘char* __builtin___strncat_chk(char*, const char*, long unsigned int, long unsigned int)’ specified 
  - /usr/include/x86_64-linux-gnu/bits/string_fortified.h:138:34: warning: ‘char* __builtin___strncat_chk(char*, const char*, long unsigned int, long unsigned int)’ specified bound 1024 equals destination size [-Wstringop-overflow=]
- [1x] writing 4 bytes into a region of size 3 [-Wstringop-overflow=]
  - /home/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shade_calc.cpp:676:24: warning: writing 4 bytes into a region of size 3 [-Wstringop-overflow=]

## macos-arm64-Debug

status: success

errors: 0
warnings: 188 in 16 kinds

### errors


### failure context


### warning kinds

- [92x] -deprecated-declarations
  - /Users/runner/work/scp-idtech3/scp-idtech3/openal-soft-1.23.1/alc/backends/coreaudio.cpp:70:9: warning: 'kAudioObjectPropertyElementMaster' is deprecated: first deprecated in macOS 12.0 [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/openal-soft-1.23.1/alc/backends/coreaudio.cpp:78:9: warning: 'kAudioObjectPropertyElementMaster' is deprecated: first deprecated in macOS 12.0 [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.h:252:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.h:266:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.h:280:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:260:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:275:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:290:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:302:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:314:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:326:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:441:8: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_vector.cpp:148:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/q_parse.cpp:110:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/q_parse.cpp:121:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:317:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:377:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:393:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:937:8: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:1005:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_main.cpp:550:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_main.cpp:561:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_shared.cpp:343:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_shared.cpp:365:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_team.cpp:95:7: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:310:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:325:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:336:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_team.cpp:956:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_draw.cpp:2678:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_draw.cpp:2680:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_draw.cpp:2685:3: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:1889:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/ai/ai_system.cpp:101:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_struct.cpp:366:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_script.cpp:254:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_script.cpp:281:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:147:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:170:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:707:3: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_ai_chat.cpp:923:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_ai_chat.cpp:931:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_interface.cpp:82:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_interface.cpp:93:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - 1/Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:377:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2441:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2467:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2547:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2577:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_model.cpp:197:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_init.cpp:567:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/external/stb/stb_image_write.h:780:13: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_bot.cpp:179:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:148:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:225:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:285:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_client.cpp:202:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_client.cpp:214:3: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/net_chan.cpp:663:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_scrn.cpp:344:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_main.cpp:191:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:2249:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1814:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1844:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1876:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3119:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3125:14: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3129:14: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3527:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/files.cpp:1521:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:315:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
- [22x] -inconsistent-missing-override
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/FxPrimitives.h:251:15: warning: 'Die' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/FxPrimitives.h:252:15: warning: 'Update' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
  - :251:15: warning: 'Die' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/FxPrimitives.h:2523:15: warning: 'Update' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
- [18x] -invalid-source-encoding
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:197:5: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:450:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:452:12: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:469:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:470:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:474:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:475:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:476:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:477:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:478:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:535:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:541:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:570:13: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:595:9: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:607:13: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:666:16: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:667:12: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:680:4: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
- [12x] -pointer-bool-conversion
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_ents.cpp:397:14: warning: address of array 'cent->dl_stylestring' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_drawtools.cpp:148:8: warning: address of array 'backgroundcolor' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/ai/ai_script_actions_combat.cpp:533:45: warning: address of array 'g_entities[cs->entityNum].client->ps.weapons' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_servercmds.cpp:948:8: warning: address of array 'text' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_servercmds.cpp:960:8: warning: address of array 'text' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_servercmds.cpp:972:8: warning: address of array 'text' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_main.cpp:1449:7: warning: address of array 'levelname' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_main.cpp:1507:8: warning: address of array 'backgroundcolor' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_weapons.cpp:1005:10: warning: address of array 'comppath' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sound/snd_dma.cpp:1857:19: warning: address of array 'ss->loop' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sound/snd_dma.cpp:2171:11: warning: address of array 'ss->loop' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sound/snd_dma.cpp:2487:15: warning: address of array 'ss->loop' will always evaluate to 'true' [-Wpointer-bool-conversion]
- [11x] -sizeof-pointer-memaccess
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_matrix.h:94:28: warning: 'memcpy' call operates on objects of type 'float[3]' while the size is based on a different type 'float (*)[3]' [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:807:29: warning: 'memset' call operates on objects of type 'int' while the size is based on a different type 'int *' [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:999:32: warning: 'memset' call operates on objects of type 'animScriptCommand_t' while the size is based on a different type 'animScriptCommand_t *' [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/ai/ai_system.cpp:458:33: warning: 'memset' call operates on objects of type 'cast_state_t' (aka 'cast_state_s') while the size is based on a different type 'cast_state_t *' (aka 'cast_state_s *') [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/cvar.cpp:764:28: warning: 'memset' call operates on objects of type 'cvar_t' (aka 'cvar_s') while the size is based on a different type 'cvar_t *' (aka 'cvar_s *') [-Wsizeof-pointer-memaccess]
- [11x] -deprecated-register
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_shared.cpp:4702:6: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3057:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3058:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3059:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3062:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3066:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:694:6: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
- [7x] -sizeof-array-argument
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_matrix.h:94:26: warning: sizeof on array function parameter will return size of 'float (*)[3]' instead of 'float[3][3]' [-Wsizeof-array-argument]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:807:27: warning: sizeof on array function parameter will return size of 'int *' instead of 'int[2]' [-Wsizeof-array-argument]
- [3x] -tautological-constant-out-of-range-compare
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_players.cpp:773:25: warning: result of comparison of constant -1 with expression of type 'weapon_t' is always true [-Wtautological-constant-out-of-range-compare]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_players.cpp:1615:21: warning: result of comparison of constant -1 with expression of type 'weapon_t' is always true [-Wtautological-constant-out-of-range-compare]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_players.cpp:1628:20: warning: result of comparison of constant -1 with expression of type 'weapon_t' is always false [-Wtautological-constant-out-of-range-compare]
- [2x] -format
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2441:31: warning: format specifies type 'int' but the argument has type 'long' [-Wformat]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2547:31: warning: format specifies type 'int' but the argument has type 'long' [-Wformat]
- [2x] -array-bounds
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shade_calc.cpp:679:2: warning: array index 3 is past the end of the array (which contains 3 elements) [-Warray-bounds]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_image.cpp:2966:59: warning: array index 3 is past the end of the array (which contains 3 elements) [-Warray-bounds]
- [2x] -enum-compare
  - /Applications/Xcode_15.4.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/sys/cdefs.h:218:48: note: expanded from macro '__deprecated_msg'/Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:986:23: warning: comparison of different enumeration types ('alphaGen_t' and 'colorGen_t') [-Wenum-compare]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:1898:26: warning: comparison of different enumeration types ('alphaGen_t' and 'colorGen_t') [-Wenum-compare]
- [2x] -int-to-pointer-cast
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:116:37: warning: cast to '__sFILE *' from smaller integer type 'fileHandle_t' (aka 'int') [-Wint-to-pointer-cast]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:120:9: warning: cast to '__sFILE *' from smaller integer type 'fileHandle_t' (aka 'int') [-Wint-to-pointer-cast]
- [1x] -macro-redefined
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_vector.cpp:40:9: warning: 'M_PI' macro redefined [-Wmacro-redefined]
- [1x] -parentheses-equality
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:2005:43: warning: equality comparison with extraneous parentheses [-Wparentheses-equality]
- [1x] -null-dereference
  - vsprintf( msg,fmt,argptr );/Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1799:2: warning: indirection of non-volatile null pointer will be deleted, not trap [-Wnull-dereference]
- [1x] -strncat-size
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:2838:25: warning: the value of the size argument in 'strncat' is too large, might lead to a buffer overflow [-Wstrncat-size]

## macos-arm64-Release

status: success

errors: 0
warnings: 189 in 17 kinds

### errors


### failure context


### warning kinds

- [92x] -deprecated-declarations
  - /Users/runner/work/scp-idtech3/scp-idtech3/openal-soft-1.23.1/alc/backends/coreaudio.cpp:70:9: warning: 'kAudioObjectPropertyElementMaster' is deprecated: first deprecated in macOS 12.0 [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/openal-soft-1.23.1/alc/backends/coreaudio.cpp:78:9: warning: 'kAudioObjectPropertyElementMaster' is deprecated: first deprecated in macOS 12.0 [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.h:252:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.h:266:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.h:280:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:260:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:275:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:290:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:302:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:314:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:326:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/util_str.cpp:441:8: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_shared.cpp:343:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_shared.cpp:365:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:317:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_drawtools.cpp:148:8: /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_vector.cpp:148:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarati
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/q_parse.cpp:110:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/q_parse.cpp:121:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:377:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:393:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_main.cpp:550:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_main.cpp:561:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:937:8: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/q_shared.cpp:1005:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/ai/ai_system.cpp:101:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:310:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:325:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:336:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_draw.cpp:2678:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_draw.cpp:2680:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_draw.cpp:2685:3: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_team.cpp:95:7: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_main.cpp:1889:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/g_team.cpp:956:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_script.cpp:254:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_script.cpp:281:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_ai_chat.cpp:923:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_ai_chat.cpp:931:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_interface.cpp:82:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Applications/Xcode_15.4.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/stdio.h:199:1: note: 'vsprintf' has been explicitly marked deprecated here/Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:147:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the 
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:170:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/be_interface.cpp:93:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_struct.cpp:366:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:707:3: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2441:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2467:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2547:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2577:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_model.cpp:197:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_init.cpp:567:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/external/stb/stb_image_write.h:780:13: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_bot.cpp:179:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_client.cpp:202:4: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_client.cpp:214:3: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/server/sv_main.cpp:191/Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/net_chan.cpp:663:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - :2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - :344:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:148:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:225:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:285:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/files.cpp:1521:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1814:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Applications/Xcode_15.4.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/sys/cdefs.h:218:/Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:2249:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use 
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1844:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1876:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3119:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3125:14: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3129:14: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:3527:2: warning: 'sprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use snprintf(3) instead. [-Wdeprecated-declarations]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:315:2: warning: 'vsprintf' is deprecated: This function is provided for compatibility reasons only.  Due to security concerns inherent in the design of sprintf(3), it is highly recommended that you use vsnprintf(3) instead. [-Wdeprecated-declarations]
- [22x] -inconsistent-missing-override
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/FxPrimitives.h:251:15: warning: 'Die' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/FxPrimitives.h:252:15: warning: 'Update' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/FxPrimitives.h:251:15: warning: 'Die' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]In file included from /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/FxScheduler.cpp:6:
  - 252:15: warning: 'Update' overrides a member function but is not marked 'override' [-Winconsistent-missing-override]
- [18x] -invalid-source-encoding
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:197:5: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_scrn.cpp/Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:450:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:452:12: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:469:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:470:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:474:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:475:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:476:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:477:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:478:7: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:535:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:541:8: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:570:13: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:595:9: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:607:13: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:666:16: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:667:12: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_keys.cpp:680:4: warning: illegal character encoding in string literal [-Winvalid-source-encoding]
- [12x] -pointer-bool-conversion
  - warning: address of array 'backgroundcolor' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_ents.cpp:397:14: warning: address of array 'cent->dl_stylestring' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_servercmds.cpp:948:8: warning: address of array 'text' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_servercmds.cpp:960:8: warning: address of array 'text' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_servercmds.cpp:972:8: warning: address of array 'text' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/cgame/cg_weapons.cpp:1005:10: warning: address of array 'comppath' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_main.cpp:1449:7: warning: address of array 'levelname' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_main.cpp:1507:8: warning: address of array 'backgroundcolor' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/ai/ai_script_actions_combat.cpp:533:45: warning: address of array 'g_entities[cs->entityNum].client->ps.weapons' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - :1857:19: warning: address of array 'ss->loop' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sound/snd_dma.cpp:2171:11: warning: address of array 'ss->loop' will always evaluate to 'true' [-Wpointer-bool-conversion]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sound/snd_dma.cpp:2487:15: warning: address of array 'ss->loop' will always evaluate to 'true' [-Wpointer-bool-conversion]
- [11x] -sizeof-pointer-memaccess
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_matrix.h:94:28: warning: 'memcpy' call operates on objects of type 'float[3]' while the size is based on a different type 'float (*)[3]' [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:807:29: warning: 'memset' call operates on objects of type 'int' while the size is based on a different type 'int *' [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:999:32: warning: 'memset' call operates on objects of type 'animScriptCommand_t' while the size is based on a different type 'animScriptCommand_t *' [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/ai/ai_system.cpp:458:33: warning: 'memset' call operates on objects of type 'cast_state_t' (aka 'cast_state_s') while the size is based on a different type 'cast_state_t *' (aka 'cast_state_s *') [-Wsizeof-pointer-memaccess]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/cvar.cpp:764:28: warning: 'memset' call operates on objects of type 'cvar_t' (aka 'cvar_s') while the size is based on a different type 'cvar_t *' (aka 'cvar_s *') [-Wsizeof-pointer-memaccess]
- [11x] -deprecated-register
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_shared.cpp:4702:6: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:694:6: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sound/snd_dma.cpp/Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3057:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3058:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3059:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3062:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:3066:3: warning: 'register' storage class specifier is deprecated and incompatible with C++17 [-Wdeprecated-register]
- [7x] -sizeof-array-argument
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_matrix.h:94:26: warning: sizeof on array function parameter will return size of 'float (*)[3]' instead of 'float[3][3]' [-Wsizeof-array-argument]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/game/bg_animation.cpp:807:27: warning: sizeof on array function parameter will return size of 'int *' instead of 'int[2]' [-Wsizeof-array-argument]
- [3x] -tautological-constant-out-of-range-compare
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_players.cpp:773:25: warning: result of comparison of constant -1 with expression of type 'weapon_t' is always true [-Wtautological-constant-out-of-range-compare]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_players.cpp:1615:21: warning: result of comparison of constant -1 with expression of type 'weapon_t' is always true [-Wtautological-constant-out-of-range-compare]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/ui/ui_players.cpp:1628:20: warning: result of comparison of constant -1 with expression of type 'weapon_t' is always false [-Wtautological-constant-out-of-range-compare]
- [2x] -format
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2441:31: warning: format specifies type 'int' but the argument has type 'long' [-Wformat]
  - /Users/runner/work/scp-idtech3/scp-idtech3/gamedll/botlib/l_precomp.cpp:2547:31: warning: format specifies type 'int' but the argument has type 'long' [-Wformat]
- [2x] -enum-compare
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:986:23: warning: comparison of different enumeration types ('alphaGen_t' and 'colorGen_t') [-Wenum-compare]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shader.cpp:1898:26: warning: comparison of different enumeration types ('alphaGen_t' and 'colorGen_t') [-Wenum-compare]
- [2x] -array-bounds
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_shade_calc.cpp:679:2: warning: array index 3 is past the end of the array (which contains 3 elements) [-Warray-bounds]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/renderer/tr_image.cpp:2966:59: warning: array index 3 is past the end of the array (which contains 3 elements) [-Warray-bounds]
- [2x] -int-to-pointer-cast
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:116:37: warning: cast to '__sFILE *' from smaller integer type 'fileHandle_t' (aka 'int') [-Wint-to-pointer-cast]
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/sys/sdl/sdl_main.cpp:120:9: warning: cast to '__sFILE *' from smaller integer type 'fileHandle_t' (aka 'int') [-Wint-to-pointer-cast]
- [1x] -macro-redefined
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/math_vector.cpp:40:9: warning: 'M_PI' macro redefined [-Wmacro-redefined]
- [1x] -unused-value
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/splines/splines.cpp:580:17: warning: expression result unused [-Wunused-value]
- [1x] -null-dereference
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/common.cpp:1799:2: warning: indirection of non-volatile null pointer will be deleted, not trap [-Wnull-dereference]
- [1x] -strncat-size
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/client/cl_main.cpp:2838:25: warning: the value of the size argument in 'strncat' is too large, might lead to a buffer overflow [-Wstrncat-size]
- [1x] -parentheses-equality
  - /Users/runner/work/scp-idtech3/scp-idtech3/engine/qcommon/unzip.cpp:2005:43: warning: equality comparison with extraneous parentheses [-Wparentheses-equality]

## windows-x86-Debug

status: success

errors: 0
warnings: 0 in 0 kinds

### errors


### failure context


### warning kinds

## windows-x86-Release

status: success

errors: 0
warnings: 0 in 0 kinds

### errors


### failure context


### warning kinds

