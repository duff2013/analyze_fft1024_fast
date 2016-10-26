Optimized version of Teensyduino Audio library's FFT algorithm.
---
The code below shows the update function for the Audio Object -> AudioAnalyzeFFT1024. The majority of the usage comes from the call to the CMSIS-DSP function ```arm_cfft_radix4_q15``` which performs the fft calculation. <br>
```C
void AudioAnalyzeFFT1024::update(void)
{
	audio_block_t *block;

	block = receiveReadOnly();
	if (!block) return;

#if defined(KINETISK)
	switch (state) {
	case 0:
		blocklist[0] = block;
		state = 1;
		break;
	case 1:
		blocklist[1] = block;
		state = 2;
		break;
	case 2:
		blocklist[2] = block;
		state = 3;
		break;
	case 3:
		blocklist[3] = block;
		state = 4;
		break;
	case 4:
		blocklist[4] = block;
		state = 5;
		break;
	case 5:
		blocklist[5] = block;
		state = 6;
		break;
	case 6:
		blocklist[6] = block;
		state = 7;
		break;
	case 7:
		blocklist[7] = block;
		// TODO: perhaps distribute the work over multiple update() ??
		//       github pull requsts welcome......
		copy_to_fft_buffer(buffer+0x000, blocklist[0]->data);
		copy_to_fft_buffer(buffer+0x100, blocklist[1]->data);
		copy_to_fft_buffer(buffer+0x200, blocklist[2]->data);
		copy_to_fft_buffer(buffer+0x300, blocklist[3]->data);
		copy_to_fft_buffer(buffer+0x400, blocklist[4]->data);
		copy_to_fft_buffer(buffer+0x500, blocklist[5]->data);
		copy_to_fft_buffer(buffer+0x600, blocklist[6]->data);
		copy_to_fft_buffer(buffer+0x700, blocklist[7]->data);
		if (window) apply_window_to_fft_buffer(buffer, window);
		arm_cfft_radix4_q15(&fft_inst, buffer);
		// TODO: support averaging multiple copies
		for (int i=0; i < 512; i++) {
			uint32_t tmp = *((uint32_t *)buffer + i); // real & imag
			uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
			output[i] = sqrt_uint32_approx(magsq);
		}
		outputflag = true;
		release(blocklist[0]);
		release(blocklist[1]);
		release(blocklist[2]);
		release(blocklist[3]);
		blocklist[0] = blocklist[4];
		blocklist[1] = blocklist[5];
		blocklist[2] = blocklist[6];
		blocklist[3] = blocklist[7];
		state = 4;
		break;
	}
#else
	release(block);
#endif
}

```
Flow Chart for Teensy Object analyze_fft1024 for the code shown above:<br>
![alt text](https://github.com/duff2013/analyze_fft1024_fast/blob/master/img/FFT%20Flow%20Chart/Slide1.png "FFT Flow Chart")
---
The code below shows the update function for the Audio Object -> AudioAnalyzeFFT1024_Fast. Note how the CMSIS-DSP calculation is now split into three stages ```arm_cfft_radix4_q15_stage(1,2,3)```. This saves significant amount of your max usage (30% savings)/Teensy 3.2 @ 96MHz. This unfortunately introduces a delay in the output of the FFT data from the AudioAnalyzeFFT1024 Object. The output is delayed by 3 Block cycles.<br>
```C
void AudioAnalyzeFFT1024_Fast::update(void)
{
    audio_block_t *block;
    
    block = receiveReadOnly();
    if (!block) return;
    
#if defined(KINETISK)
    int16_t *buf = buffer;
    switch (state) {
        case 0:
            blocklist[0] = block;
            state = 1;
            break;
        case 1:
            blocklist[1] = block;
            state = 2;
            break;
        case 2:
            blocklist[2] = block;
            state = 3;
            break;
        case 3:
            blocklist[3] = block;
            state = 4;
            break;
        case 4:
            blocklist[4] = block;
            // stage 2 of the fft algorithm
            arm_cfft_radix4_q15_stage2(&fft_inst, buf);
            state = 5;
            break;
        case 5:
            blocklist[5] = block;
            // stage 3 of the fft algorithm
            arm_cfft_radix4_q15_stage3(&fft_inst, buf);
            // TODO: support averaging multiple copies
            for (int i=0; i < 512; i++) {
                uint32_t tmp = *((uint32_t *)buf + i); // real & imag
                uint32_t magsq = multiply_16tx16t_add_16bx16b(tmp, tmp);
                output[i] = sqrt_uint32_approx(magsq);
            }
            outputflag = true;
            state = 6;
            break;
        case 6:
            blocklist[6] = block;
            state = 7;
            break;
        case 7:
            blocklist[7] = block;
            // TODO: perhaps distribute the work over multiple update() ??
            //       github pull requsts welcome......
            copy_to_fft_buffer(buf+0x000, blocklist[0]->data);
            copy_to_fft_buffer(buf+0x100, blocklist[1]->data);
            copy_to_fft_buffer(buf+0x200, blocklist[2]->data);
            copy_to_fft_buffer(buf+0x300, blocklist[3]->data);
            copy_to_fft_buffer(buf+0x400, blocklist[4]->data);
            copy_to_fft_buffer(buf+0x500, blocklist[5]->data);
            copy_to_fft_buffer(buf+0x600, blocklist[6]->data);
            copy_to_fft_buffer(buf+0x700, blocklist[7]->data);
            if (window) apply_window_to_fft_buffer(buf, window);
            // stage 1 of the fft algorithm
            arm_cfft_radix4_q15_stage1(&fft_inst, buf);
            release(blocklist[0]);
            release(blocklist[1]);
            release(blocklist[2]);
            release(blocklist[3]);
            blocklist[0] = blocklist[4];
            blocklist[1] = blocklist[5];
            blocklist[2] = blocklist[6];
            blocklist[3] = blocklist[7];
            state = 4;
            break;
    }
#else
    release(block);
#endif
}
```
Flow Chart for Teensy Object analyze_fft1024_fast for the code shown above:<br>
![alt text](https://github.com/duff2013/analyze_fft1024_fast/blob/master/img/FFT%20Flow%20Chart/Slide2.png "FFT Flow Chart")

[analyze_fft1024]:https://github.com/duff2013/analyze_fft1024_fast/blob/master/analyze_fft1024_fast.cpp#L62

[analyze_fft1024_fast]:https://github.com/PaulStoffregen/Audio/blob/master/analyze_fft1024.cpp#L57
