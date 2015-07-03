package edu.ucdavis.fiehnlab.spectra.hash.core.io;

import edu.ucdavis.fiehnlab.spectra.hash.core.Spectrum;
import org.junit.Test;

import java.io.StringReader;
import static org.junit.Assert.*;

/**
 * simple test of a spectrum reader
 */
public class SpectrumReaderTest {

    Spectrum spectrum = null;

    @Test
    public void testReadSpectrum() throws Exception {

        SpectrumReader reader = new SpectrumReader();

        reader.readSpectrum(new StringReader("100:1 101:2 103:3"), new SpectraHandler() {
            public void handle(Spectrum s) {
                spectrum = s;
            }
        });

        assertNotNull(spectrum);


    }
}