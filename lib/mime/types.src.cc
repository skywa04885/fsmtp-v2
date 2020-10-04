/*
	Copyright [2020] [Luke A.C.A. Rieff]

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#include "types.src.h"

namespace FSMTP::MIME {
  const char *__fileTypeToString(FileTypes type) {
    assert(type > FileTypes::FileTypesStart && type < FileTypes::FileTypesEnd);

    switch (type) {
			case AACAudio: return "audio/aac";
			case AbiWord: return "application/x-abiword";
			case FreeArchive: return "application/x-freearc";
			case AviAudio: return "video/x-msvideo";
			case KindleEbook: return "application/vnd.amazon.ebook";
			case Binary: return "application/octet-stream";
			case BitmapGraphics: return "image/bmp";
			case BzipArchive: return "application/x-bzip";
			case Bzip2Archive: return "application/x-bzip2";
			case CShellScript: return "application/x-csh";
			case CascadingStyleSheets: return "text/css";
			case CommaSeparatedValues: return "text/csv";
			case MicrosoftWord: return "application/msword";
			case MicrosoftWordOpenXML: return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
			case MSOpenTypeFonts: return "application/vnd.ms-fontobject";
			case ElectronicPublication: return "application/epub+zip";
			case GZIPArchive: return "application/gzip";
			case AnimatedGIF: return "image/gif";
			case HypertextMarkupLanguage: return "text/html";
			case IconFormat: return "image/vnd.microsoft.icon";
			case IcalenderFormat: return "text/calendar";
			case JavaArchive: return "application/java-archive";
			case JPEGImage: return "image/jpeg";
			case JavaScript: return "text/javascript";
			case JSON: return "application/json";
			case JSON_LD: return "application/ld+json";
			case MusicalInstrumentDigitalInterface: return "audio/x-midi";
			case JavaScriptModule: return "text/javascript";
			case MP3Audio: return "audio/mpeg";
			case MPEGVideo: return "video/mpeg";
			case AppleInstallerPackage: return "application/vnd.apple.installer+xml";
			case OpenDocumentPresentation: return "application/vnd.oasis.opendocument.presentation";
			case OpenDocumentSpreadSheet: return "application/vnd.oasis.opendocument.spreadsheet";
			case OpenDocumentText: return "application/vnd.oasis.opendocument.text";
			case OGGAudio: return "audio/ogg";
			case OGGVideo: return "video/ogg";
			case OpusAudio: return "audio/opus";
			case OpenTypeFont: return "font/otf";
			case PortableNetworkGraphics: return "image/png";
			case AdobePortableDocumentFormat: return "application/pdf";
			case HypertextPreprocessor: return "application/x-httpd-php";
			case MicrosoftPowerpoint: return "application/vnd.ms-powerpoint";
			case MicrosoftPowerpointOpenXML: return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
			case RARArchive: return "application/vnd.rar";
			case RichTextFormat: return "application/rtf";
			case BourneShellScript: return "application/x-sh";
			case ScalableVectorGraphics: return "image/svg+xml";
			case SmallWebFormat: return "application/x-shockwave-flash";
			case TapeArchive: return "application/x-tar";
			case TaggedImage: return "image/tiff";
			case MPEGTransportStream: return "video/mp2t";
			case TrueTypeFont: return "font/ttf";
			case Text: return "text/plain";
			case MicrosoftVisio: return "application/vnd.visio";
			case WaveformAudio: return "audio/wav";
			case WEBMAudio: return "audio/webm";
			case WEBMVideo: return "video/webm";
			case WEBMImage: return "image/webp";
			case WebOpenFontFormat: return "font/woff";
			case WepOpenFontFormat2: return "font/woff2";
			case XHTML: return "application/xhtml+xml";
			case MicrosoftExcel: return "application/vnd.ms-excel";
			case MicrosoftExcelOpenXML: return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
			case XML: return "application/xml";
			case XUL: return "application/vnd.mozilla.xul+xml";
			case ZIPArchive: return "application/zip";
			case ThreeGP: return "video/3gpp";
			case ThreeGP2: return "video/3gpp2";
			case SevenZipArchive: return "application/x-7z-compressed";
      default: return nullptr;
    }
  }

  FileTypes __extensionToFileType(string ext) {
		static map<string, FileTypes> __exts = {
			make_pair(".aac", AACAudio),
			make_pair(".abw", AbiWord),
			make_pair(".arc", FreeArchive),
			make_pair(".avi", AviAudio),
			make_pair(".azw", KindleEbook),
			make_pair(".bin", Binary),
			make_pair(".bmp", BitmapGraphics),
			make_pair(".bz", BzipArchive),
			make_pair(".bz2", Bzip2Archive),
			make_pair(".csh", CShellScript),
			make_pair(".css", CascadingStyleSheets),
			make_pair(".csv", CommaSeparatedValues),
			make_pair(".doc", MicrosoftWord),
			make_pair(".docx", MicrosoftWordOpenXML),
			make_pair(".eot", MSOpenTypeFonts),
			make_pair(".epub", ElectronicPublication),
			make_pair(".gz", GZIPArchive),
			make_pair(".gif", AnimatedGIF),
			make_pair(".html", HypertextMarkupLanguage),
			make_pair(".htm", HypertextMarkupLanguage),
			make_pair(".ico", IconFormat),
			make_pair(".ics", IcalenderFormat),
			make_pair(".jar", JavaArchive),
			make_pair(".jpeg", JPEGImage),
			make_pair(".js", JavaScript),
			make_pair(".json", JSON),
			make_pair(".jsonld", JSON_LD),
			make_pair(".midi", MusicalInstrumentDigitalInterface),
			make_pair(".mid", MusicalInstrumentDigitalInterface),
			make_pair(".mjs", JavaScriptModule),
			make_pair(".mp3", MP3Audio),
			make_pair(".mpeg", MPEGVideo),
			make_pair(".mpkg", AppleInstallerPackage),
			make_pair(".odp", OpenDocumentPresentation),
			make_pair(".ods", OpenDocumentSpreadSheet),
			make_pair(".odt", OpenDocumentText),
			make_pair(".ogg", OGGAudio),
			make_pair(".ogv", OGGVideo),
			make_pair(".opus", OpusAudio),
			make_pair(".otf", OpenTypeFont),
			make_pair(".png", PortableNetworkGraphics),
			make_pair(".pdf", AdobePortableDocumentFormat),
			make_pair(".php", HypertextPreprocessor),
			make_pair(".ppt", MicrosoftPowerpoint),
			make_pair(".pptx", MicrosoftPowerpointOpenXML),
			make_pair(".rar", RARArchive),
			make_pair(".rtf", RichTextFormat),
			make_pair(".sh", BourneShellScript),
			make_pair(".svg", ScalableVectorGraphics),
			make_pair(".swf", SmallWebFormat),
			make_pair(".tar", TapeArchive),
			make_pair(".tif", TaggedImage),
			make_pair(".ts", MPEGTransportStream),
			make_pair(".ttf", TrueTypeFont),
			make_pair(".txt", Text),
			make_pair(".vsd", MicrosoftVisio),
			make_pair(".wav", WaveformAudio),
			make_pair(".weba", WEBMAudio),
			make_pair(".webm", WEBMVideo),
			make_pair(".webp", WEBMImage),
			make_pair(".woff", WebOpenFontFormat),
			make_pair(".woff2", WepOpenFontFormat2),
			make_pair(".xhtml", XHTML),
			make_pair(".xls", MicrosoftExcel),
			make_pair(".xlsx", MicrosoftExcelOpenXML),
			make_pair(".xml", XML),
			make_pair(".xul", XUL),
			make_pair(".zip", ZIPArchive),
			make_pair(".3gp", ThreeGP),
			make_pair(".3g2", ThreeGP2),
			make_pair(".7z", SevenZipArchive)
		};

		transform(ext.begin(), ext.end(), ext.begin(), [&](const char c) {
			return tolower(c);
		});

		auto it = __exts.find(ext);
		if (it == __exts.end()) throw runtime_error(
			EXCEPT_DEBUG("Invalid extension: '" + ext + '\''));
		return it->second;
	}
}