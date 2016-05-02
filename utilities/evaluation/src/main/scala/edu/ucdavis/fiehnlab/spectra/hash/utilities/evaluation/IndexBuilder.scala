package edu.ucdavis.fiehnlab.spectra.hash.utilities.evaluation

import edu.ucdavis.fiehnlab.util.MathUtilities
import edu.ucdavis.fiehnlab._
import edu.ucdavis.fiehnlab.index._
import edu.ucdavis.fiehnlab.index.cache.SpectrumCache
import edu.ucdavis.fiehnlab.index.histogram.{SimilarHistogramIndex, HistogramIndex}
import edu.ucdavis.fiehnlab.math.histogram._
import edu.ucdavis.fiehnlab.math.histogram.SplashHistogram._

import edu.ucdavis.fiehnlab.math.spectrum.{BinByRoundingMethod, BinningMethod}
import org.springframework.beans.factory.annotation.Autowired
import org.springframework.stereotype.Component
import edu.ucdavis.fiehnlab.math.histogram.Base64Histogram._

/**
  * Created by wohlgemuth on 2/19/16.
  */
@Component
class IndexBuilder {

  val binniningMethod: BinningMethod = new BinningMethod {
    override def binSpectrum(spectrum: Spectrum): Spectrum = {

      def round(ion: Ion): Ion = new Ion(Math.floor(ion.mz))

      new Spectrum(spectrum.splash, MathUtilities.binFragments(spectrum.peaklist, round), spectrum.id, spectrum.origin) with SpectrumIsBinned
    }
  }

  val spectraCache: SpectrumCache = SpectrumCache.create()

  def buildLinear: Index = {
    new LinearIndex(binniningMethod, spectraCache)
  }

  /**
    * a list of predifined indexes to utilize
    *
    * @return
    */
  def build(): List[Index] = {

    val histogramList: List[Histogram] =
      new Top10IonsSeparationHistogram ::
        new Top10IonsModulo36Histogram ::
        Seq(8, 10, 16, 36).collect {
          case base =>

            Seq(10, 15, 20, 25).collect {
              case length =>

                Seq(5, 10, 25, 50, 75, 100).collect {

                  case bin =>
                    new SplashHistogram(base, length, bin)
                }
            }.flatten
        }.flatten.toList

    val histogramBasedIndex: List[Index] = histogramList.collect {
      case histogram => new HistogramIndex(binniningMethod, spectraCache, histogram)
    }.toList

    val indexList = new LinearIndex(binniningMethod, spectraCache) :: histogramBasedIndex

    indexList
  }

  /**
    * based on these ones
    *
    * 8|100|20 8035 11 7993 11

    * 8|100|25 8035 11 7993 11

    * 8|100|15 8034 12 7992 12

    * 8|100|10 8032 14 7990 14

    * 10|100|10 8029 17 7987 17

    * 10|100|15 8024 22 7982 22

    * 10|100|20 8023 23 7981 23

    * 10|100|25 8023 23 7981 23

    * which were considered best and these will be compared against larger histograms

    * 125
    * 150
    * 250
    *
    * @return
    */
  def buildBestIndexes: List[Index] = {

    val histogramList: List[Histogram] =  LongBase64NoToleranceHistogram :: LongBase64SmallToleranceHistogram :: LongBase64MediumToleranceHistogram :: LongBase64LargeToleranceHistogram:: ShortBase64NoToleranceHistogram :: ShortBase64SmallToleranceHistogram ::
      Seq(8, 10).collect {
        case base =>

          Seq(10, 15, 20, 25).collect {
            case length =>

              Seq(100, 125, 150, 250).collect {
                case bin =>
                  new SplashHistogram(base, length, bin)
              }
          }.flatten
      }.flatten.toList

    val histogramBasedIndex: List[Index] = histogramList.collect {
      case histogram => new HistogramIndex(binniningMethod, spectraCache, histogram)
    }.toList

    val indexList = new LinearIndex :: histogramBasedIndex

    indexList
  }

}
