/**
 * OLBikMovieTexture
 * TextureMovie subclass with Bink decoder and looping enabled by default.
 * Used by OLBikScreen for runtime .bik file playback.
 */
class OLBikMovieTexture extends TextureMovie;

defaultproperties
{
    DecoderClass=class'CodecMovieBink'
    Looping=true
    AutoPlay=false
    MovieStreamSource=MovieStream_Memory
}
