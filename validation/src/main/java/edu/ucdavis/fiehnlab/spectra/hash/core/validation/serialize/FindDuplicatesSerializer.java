package edu.ucdavis.fiehnlab.spectra.hash.core.validation.serialize;

import edu.ucdavis.fiehnlab.spectra.hash.core.validation.controller.ValidationController;
import org.apache.commons.cli.CommandLine;

import java.io.IOException;
import java.io.PrintStream;

/**
 * a serializer to find duplicates
 */
public class FindDuplicatesSerializer extends SortSerializer {

    private Result last = null;

    private boolean wroteLast = false;

    private long duplicatesFound = 0;

    @Override
    public void init() throws Exception {
        super.init();
        last = null;
        wroteLast = false;
        duplicatesFound = 0;
    }

    @Override
    protected void serializeSortedData(Result sortedData) {
        if (last == null) {
            last = sortedData;
        } else {
            if (sortedData.equals(last)) {
                if (!wroteLast) {
                    super.serializeSortedData(last);
                    wroteLast = true;
                }

                duplicatesFound++;
                ValidationController.status(getCmd(), String.format(ValidationController.FORMAT, "duplicated splash: ") + sortedData.getSplash() + "\n");
                ValidationController.status(getCmd(), String.format(ValidationController.FORMAT, "duplicated origin: ") + sortedData.getOrigin() + "\n");

                //duplicated, write out
                super.serializeSortedData(sortedData);
            }
        }

    }

    @Override
    public void close() throws IOException {
        super.close();
        ValidationController.status(getCmd(), "duplicates found: " + duplicatesFound + "\n");
    }

    public FindDuplicatesSerializer(CommandLine cmd, PrintStream stream, Class<? extends Result> type) throws Exception {
        super(cmd, stream, type);
    }
}
