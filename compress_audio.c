#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <vorbis/vorbisenc.h>
#include <sndfile.h>

#define READ 1024

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input_file.wav> [output_file.ogg]\n", argv[0]);
        return 1;
    }

    const char *input_filename = argv[1];
    const char *output_filename = (argc > 2) ? argv[2] : "output.ogg";

    SF_INFO sfinfo;
    SNDFILE *infile = sf_open(input_filename, SFM_READ, &sfinfo);
    if (!infile) {
        fprintf(stderr, "Error: Could not open input WAV file: %s\n", input_filename);
        return 1;
    }

    printf("Input file has %d channels, %d sample rate.\n", sfinfo.channels, sfinfo.samplerate);

    ogg_stream_state os;
    ogg_page og;
    ogg_packet op;
    vorbis_info vi;
    vorbis_comment vc;
    vorbis_dsp_state vd;
    vorbis_block vb;
    int eos = 0, ret;

    vorbis_info_init(&vi);
    // Thiết lập chất lượng nén (VBR). Giá trị từ -1 (thấp nhất) đến 1 (cao nhất).
    float quality = 0.5;
    ret = vorbis_encode_init_vbr(&vi, sfinfo.channels, sfinfo.samplerate, quality);
    if (ret) {
        fprintf(stderr, "Error: vorbis_encode_init_vbr failed with code %d.\n", ret);
        vorbis_info_clear(&vi);
        sf_close(infile);
        return 1;
    }

    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "audio_compressor_example");

    vorbis_analysis_init(&vd, &vi);
    vorbis_block_init(&vd, &vb);

    ogg_stream_init(&os, (unsigned int)time(NULL));

    ogg_packet header, header_comm, header_code;
    vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
    ogg_stream_packetin(&os, &header);
    ogg_stream_packetin(&os, &header_comm);
    ogg_stream_packetin(&os, &header_code);

    FILE *outfile = fopen(output_filename, "wb");
    if (!outfile) {
        fprintf(stderr, "Error: Could not open output OGG file: %s\n", output_filename);
        ogg_stream_clear(&os);
        vorbis_block_clear(&vb);
        vorbis_dsp_clear(&vd);
        vorbis_comment_clear(&vc);
        vorbis_info_clear(&vi);
        sf_close(infile);
        return 1;
    }

    ogg_page header_og;
    while ((ret = ogg_stream_flush(&os, &header_og))) {
        fwrite(header_og.header, 1, header_og.header_len, outfile);
        fwrite(header_og.body, 1, header_og.body_len, outfile);
    }

    short readbuffer[READ * sfinfo.channels];
    while (!eos) {
        long frames = sf_readf_short(infile, readbuffer, READ);
        if (frames == 0) {
            vorbis_analysis_wrote(&vd, 0);
        } else {
            float **buffer = vorbis_analysis_buffer(&vd, READ);
            for (long i = 0; i < frames; i++) {
                for (int j = 0; j < sfinfo.channels; j++) {
                    buffer[j][i] = (float)readbuffer[i * sfinfo.channels + j] / 32768.0f;
                }
            }
            vorbis_analysis_wrote(&vd, frames);
        }

        while (vorbis_analysis_blockout(&vd, &vb) == 1) {
            vorbis_analysis(&vb, NULL);
            vorbis_bitrate_addblock(&vb);

            while (vorbis_bitrate_flushpacket(&vd, &op)) {
                ogg_stream_packetin(&os, &op);
                while (ogg_stream_pageout(&os, &og)) {
                    fwrite(og.header, 1, og.header_len, outfile);
                    fwrite(og.body, 1, og.body_len, outfile);
                    if (ogg_page_eos(&og)) eos = 1;
                }
            }
        }
    }

    while (ogg_stream_flush(&os, &og)) {
        fwrite(og.header, 1, og.header_len, outfile);
        fwrite(og.body, 1, og.body_len, outfile);
    }

    ogg_stream_clear(&os);
    vorbis_block_clear(&vb);
    vorbis_dsp_clear(&vd);
    vorbis_comment_clear(&vc);
    vorbis_info_clear(&vi);
    fclose(outfile);
    sf_close(infile);

    fprintf(stderr, "Compression done. Output file: %s\n", output_filename);
    return 0;
}