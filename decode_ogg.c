#include <stdio.h>
#include <stdlib.h>
#include <vorbis/vorbisfile.h>

void write_wav_header(FILE *outfile, int sample_rate, int num_channels, long num_samples) {
    unsigned char header[44] = {0};

    // "RIFF" Chunk
    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';

    unsigned int chunk_size = 36 + num_samples * num_channels * 2;
    header[4] = (chunk_size & 0xFF);
    header[5] = ((chunk_size >> 8) & 0xFF);
    header[6] = ((chunk_size >> 16) & 0xFF);
    header[7] = ((chunk_size >> 24) & 0xFF);

    // "WAVE" format
    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'V';
    header[11] = 'E';

    // "fmt " Chunk
    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';

    // Subchunk size (16 for PCM)
    header[16] = 16;

    // Audio format (1 for PCM)
    header[20] = 1;
    header[21] = 0;

    // Number of channels
    header[22] = num_channels;
    header[23] = 0;

    // Sample rate
    header[24] = (sample_rate & 0xFF);
    header[25] = ((sample_rate >> 8) & 0xFF);
    header[26] = ((sample_rate >> 16) & 0xFF);
    header[27] = ((sample_rate >> 24) & 0xFF);

    // Byte rate
    unsigned int byte_rate = sample_rate * num_channels * 2;
    header[28] = (byte_rate & 0xFF);
    header[29] = ((byte_rate >> 8) & 0xFF);
    header[30] = ((byte_rate >> 16) & 0xFF);
    header[31] = ((byte_rate >> 24) & 0xFF);

    // Block align
    header[32] = num_channels * 2;
    header[33] = 0;

    // Bits per sample (16-bit PCM)
    header[34] = 16;
    header[35] = 0;

    // "data" Chunk
    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';

    // Subchunk size
    unsigned int data_chunk_size = num_samples * num_channels * 2;
    header[40] = (data_chunk_size & 0xFF);
    header[41] = ((data_chunk_size >> 8) & 0xFF);
    header[42] = ((data_chunk_size >> 16) & 0xFF);
    header[43] = ((data_chunk_size >> 24) & 0xFF);

    // Write the header
    fwrite(header, 1, 44, outfile);
}

void decode_vorbis(const char* input_file, const char* output_file) {
    FILE *infile = fopen(input_file, "rb");
    if (!infile) {
        fprintf(stderr, "Error opening input file\n");
        return;
    }

    FILE *outfile = fopen(output_file, "wb");
    if (!outfile) {
        fprintf(stderr, "Error opening output file\n");
        fclose(infile);
        return;
    }

    OggVorbis_File vf;
    if (ov_open(infile, &vf, NULL, 0) < 0) {
        fprintf(stderr, "Error opening Vorbis file\n");
        fclose(infile);
        fclose(outfile);
        return;
    }

    vorbis_info* vi = ov_info(&vf, -1);
    int sample_rate = vi->rate;
    int num_channels = vi->channels;

    long num_samples = ov_pcm_total(&vf, -1);
    write_wav_header(outfile, sample_rate, num_channels, num_samples);

    int bitstream;
    long bytes_read;
    char buffer[4096];

    while ((bytes_read = ov_read(&vf, buffer, sizeof(buffer), 0, 2, 1, &bitstream)) > 0) {
        fwrite(buffer, 1, bytes_read, outfile);
    }

    
    fclose(infile);
    fclose(outfile);

    printf("Decoding completed!\n");
}

int main() {
    decode_vorbis("output.ogg", "output.wav");
    return 0;
}
