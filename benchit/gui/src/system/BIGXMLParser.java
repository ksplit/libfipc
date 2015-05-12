/******************************************************************************
 * B e n c h I T - Performance Measurement for Scientific Applications BIGXMLParser.java Author:
 * SWTP Nagel 1 Last change by: $Author: tschuet $ $Revision: 1.5 $ $Date: 2008/06/02 10:10:37 $
 ******************************************************************************/
package system;

import gui.DetailLevel;

import java.io.IOException;
import java.net.URL;
import java.util.*;

import org.xml.sax.*;
import org.xml.sax.helpers.XMLReaderFactory;

// import org.apache.crimson.parser.*;

/**
 * the class that should handle the XML-configfile. it should only be used by BIGInterface
 * 
 * @author <a href="mailto:pisi@pisi.de">Christoph Mueller</a>
 * @see BIGInterface
 */
public class BIGXMLParser {

	private boolean debug = false;
	private final BIGInterface bi;
	private final BIGObservable progress;
	private String filename;
	private int min, max;
	private double difference, current;
	BIGStrings out;

	/**
	 * construct a new BIGXMLParser object this should only be done by BIGInterface
	 */
	public BIGXMLParser() {
		out = new BIGStrings();
		bi = BIGInterface.getInstance();
		progress = new BIGObservable();
		filename = new String();
		// if debug is set for "BIGXMLParser" set local debug to true
		if (bi.getDebug("BIGXMLParser") > 0) {
			debug = true;
		}
	}

	/**
	 * write the current status of the interface-data into a XML file this method should only be used by BIGInterface
	 * 
	 * @param filename the file to write to
	 * @return the Thread that does the writing.
	 * @throws BIGParserException
	 */
	public Thread write(String filename) throws BIGParserException {
		progress.setProgress(0);
		this.filename = filename;
		out = new BIGStrings();
		BIGWriteThread writer = new BIGWriteThread();
		writer.start();
		return writer;
	}

	/**
	 * a shortcut processing the content of a file is called from write() as often as file-entries exist
	 * 
	 * @param it an iterator over the Entries of this file
	 * @param file the filename
	 * @return
	 */
	private void helper(Iterator<?> it) {

		BIGEntry ent;

		while (it.hasNext()) {
			ent = (BIGEntry) it.next();
			/*
			 * System.out.println( "ent: " + ent + ", POS=" + ent.getPos() + "\n" ); System.out.flush();
			 */
			switch (ent.getType()) {

				case None :
					out.add("\t\t<none");
					// out.add( "\t\t\tDEFAULTVALUE=\"";
					// out.add( ent.getDefaultValue() + "\"");
					innerhelper(ent);
					out.add("\t\t></none>");
					break;

				case String :
					out.add("\t\t<string");
					out.add("\t\t\tDEFAULTVALUE=\"" + ent.getDefaultValue() + "\"");
					innerhelper(ent);
					out.add("\t\t></string>");
					break;

				case Integer :
					out.add("\t\t<integer");
					out.add("\t\t\tDEFAULTVALUE=\"" + ent.getDefaultValue() + "\"");
					innerhelper(ent);
					out.add("\t\t></integer>");
					break;

				case Float :
					out.add("\t\t<float");
					out.add("\t\t\tDEFAULTVALUE=\"" + ent.getDefaultValue() + "\"");
					innerhelper(ent);
					out.add("\t\t></float>");
					break;

				case Boolean :
					out.add("\t\t<boolean");
					out.add("\t\t\tDEFAULTVALUE=\"" + ent.getDefaultValue() + "\"");
					innerhelper(ent);
					out.add("\t\t></boolean>");
					break;

				case Multiple :
					out.add("\t\t<multiple");
					// we have to look, which id has defaultvalue
					Iterator<String> itStr = ent.getMultipleChoice().iterator();
					int id = 0;
					int dId = -2;
					while (itStr.hasNext()) {
						String s = itStr.next();
						if (s.equals(ent.getDefaultValue())) {
							dId = id;
						}
						id++;
					}
					out.add("\t\t\tDEFAULTVALUE=\"" + dId + "\"");
					innerhelper(ent);
					out.add("\t\t>");

					// the parts
					// first reset iterator and id
					itStr = ent.getMultipleChoice().iterator();
					id = 0;
					while (itStr.hasNext()) {
						String s = itStr.next();
						out.add("\t\t\t<multiple_part");
						out.add("\t\t\t\tID=\"" + id + "\"");
						out.add("\t\t\t\tVALUE=\"" + s + "\"");
						out.add("\t\t\t></multiple_part>");
						id++;
					}
					out.add("\t\t</multiple>");
					break;

				case List :
					BIGStrings bs = (BIGStrings) ent.getDefaultValue();
					BIGStrings bs1 = ent.getMultipleChoice();
					Iterator<?> listIt = bs.iterator();

					// remove all selected entries from multiplechoice
					while (listIt.hasNext()) {
						bs1.remove(listIt.next());
					}

					out.add("\t\t<list");
					innerhelper(ent);

					out.add("\t\t\tDEFAULTVALUE=\"" + bs.getCommaText().replaceAll("\"", "&quot;") + "\"");

					out.add("\t\t\tUNSELECTED=\"" + bs1.getCommaText().replaceAll("\"", "&quot;") + "\"");
					out.add("\t\t></list>");

					break;

				case FreeText :
					out.add("\t\t<freetext");
					out.add("\t\t\tDEFAULTVALUE=\"" + ent.getDefaultValue() + "\"");
					innerhelper(ent);
					out.add("\t\t></freetext>");
					break;

				case RestrictedText :
					out.add("\t\t<restrictedtext");
					out.add("\t\t\tDEFAULTVALUE=\"" + ent.getDefaultValue() + "\"");
					innerhelper(ent);
					out.add("\t\t></restrictedtext>");
					break;

				default :
					if (debug) {
						System.out.println("BIGXMLParser: warning trying to write"
								+ " an entry with unsupported type: \"" + ent.getName() + "\"");
					}
					break;
			}

			current = current + difference;
			progress.setProgress((int) current);
			try {
				Thread.sleep(1);
			} catch (InterruptedException e) {}
		}
	}

	/**
	 * this belongs to helper()
	 */
	private void innerhelper(BIGEntry ent) {
		// all these properties, which are typeindependent
		out.add("\t\t\tPOS=\"" + ent.getPos() + "\"");
		out.add("\t\t\tMultipleCHOICECOUNT=\"" + ent.getMultipleChoiceCount() + "\"");
		out.add("\t\t\tNAME=\"" + ent.getName() + "\"");
		out.add("\t\t\tACTIVESTATUS=\"" + ent.getActiveStatus() + "\"");
		out.add("\t\t\tVIEWNAME=\"" + ent.getViewName() + "\"");
		out.add("\t\t\tTOOLTIPTEXT=\"" + ent.getToolTipText() + "\"");
		out.add("\t\t\tHELP=\"" + ent.getHelp() + "\"");
		out.add("\t\t\tACTION=\"" + ent.getAction() + "\"");
		out.add("\t\t\tDETAILLEVEL=\"" + ent.getDetailLevel() + "\"");
		out.add("\t\t\tNECESSITY=\"" + ent.getNecessity() + "\"");
	}

	public Observable getObservable() {
		return progress;
	}

	/**
	 * restore the saved status of the interface-data from a XML file this method should only be used by BIGInterface
	 * 
	 * @param filename the file to read from
	 * @throws BIGParserException
	 */
	public void read(URL filename, boolean firstTime) throws BIGParserException {
		if (debug) {
			System.out.println("BIGXMLParser: Parsing XML File: " + filename + "\n\n");
		}
		BIGContentHandler coha = new BIGContentHandler();
		coha.setFirstTime(firstTime);
		try {
			XMLReader parser;
			try { // Xerces
				parser = XMLReaderFactory.createXMLReader("org.apache.xerces.parsers.SAXParser");
			} catch (SAXException e1) {
				try { // Crimson
					parser = XMLReaderFactory.createXMLReader("org.apache.crimson.parser.XMLReaderImpl");
				} catch (SAXException e2) {
					try { // Aelfred
						parser = XMLReaderFactory.createXMLReader("gnu.xml.aelfred2.XmlReader");
					} catch (SAXException e3) {
						try { // Piccolo
							parser = XMLReaderFactory.createXMLReader("com.bluecast.xml.Piccolo");
						} catch (SAXException e4) {
							try { // Oracle
								parser = XMLReaderFactory.createXMLReader("oracle.xml.parser.v2.SAXParser");
							} catch (SAXException e5) {
								try { // default
									parser = XMLReaderFactory.createXMLReader();
								} catch (SAXException e6) {
									System.err.println("No SAX parser is available");
									System.exit(-1);
									throw new BIGParserException("No SAX parser is available");
									// or whatever exception your method is
									// declared to throw
								}
							}
						}
					}
				}
			}

			parser.setContentHandler(coha);
			parser.parse(new InputSource(filename.openStream()));
		} catch (IOException e) {
			String err = "BIGXMLParser: Error reading file \"" + filename + "\":\n" + e.getMessage();
			if (debug) {
				System.out.println(err);
			}
			throw new BIGParserException(err);
		} catch (SAXException e) {
			String err = "BIGXMLParser: Error parsing file \"" + filename + "\":\n" + e.getMessage()
					+ "\tException:\t" + e + "\tCause:\t" + e.getCause();
			if (debug) {
				System.out.println(err);
			}
			throw new BIGParserException(err);
		}
	}

	// print n times "\t"
	private String tabs(int n) {
		String out = new String();
		if (n < 1) {
			n = 0;
		}
		for (int i = 0; i < n; i++) {
			out = out.concat("\t");
		}
		return out;
	}

	/**
	 * this is our SAX Content Handler
	 * 
	 * @author Christoph Mueller
	 */
	private class BIGContentHandler implements ContentHandler {

		double howMany = 100000;
		boolean firstTime = false;

		public void setFirstTime(boolean b) {
			firstTime = b;
		}

		// store a reference to our BIGInterface
		private final BIGInterface bi;

		// store all existing entries for the actual file
		// private ArrayList entries;
		// is used for SAX (can get line and column where event occurs)
		private Locator locator;

		// store the file processing
		// is empty if we are not inside a section for a file
		String file = new String();

		// this marks whether we are inside the <bigdatasection> or not
		boolean bigdata = false;

		// this is for formatted debug output
		private int depth = 0;

		// temporary values for processing multiple
		int iMultipleD; // integer multiple defaultvalue
		// -1 means: found and processed (the value)
		// -2 means 'not recognized as int' in <multiple DEFAULTVALUE=
		// -3 means 'not recognized as int' in <multiple_part ID=

		BIGStrings bs1, bs2;

		BIGEntry eMultiple;

		/**
		 * constructs a new BIGContentHandler for handling the SAX parsing events
		 */
		public BIGContentHandler() {
			bi = BIGInterface.getInstance();
		}

		/**
		 * method used by SAX
		 */
		public void setDocumentLocator(Locator locator) {
			this.locator = locator;
		}

		/**
		 * method used by SAX the parser tells us, that it starts parsing
		 */
		public void startDocument() throws SAXException {
			if (debug) {
				System.out.println("BIGXMLParser: start parsing");
			}
		}

		/**
		 * method used by SAX the parser tells us, that it has finished
		 */
		public void endDocument() throws SAXException {
			if (debug) {
				System.out.println("BIGXMLParser: finished parsing");
			}
		}

		/**
		 * method used by SAX the parser tells us, that it has found a processing instruction
		 */
		public void processingInstruction(String target, String data) throws SAXException {}

		/**
		 * method used by SAX the parser tells us, that it has found a prefix start
		 */
		public void startPrefixMapping(String prefix, String uri) {}

		/**
		 * method used by SAX the parser tells us, that it has found a prefix end
		 */
		public void endPrefixMapping(String prefix) {}

		/**
		 * method used by SAX the parser tells us, that it has found an element start <...>
		 */
		public void startElement(String namespaceURI, String localName, String rawName, Attributes atts)
				throws SAXException {

			int i = 0;
			Integer in;
			Double d;
			BIGEntry ent;
			Boolean b;
			BIGType type = BIGType.None;
			if (debug) {
				System.out
						.print("BIGXMLParser: " + locator.getLineNumber() + tabs(depth) + "<" + localName);
				if (atts.getIndex("NAME") > -1) {
					System.out.print(" NAME=" + atts.getValue("NAME"));
				}
				if (atts.getIndex("POS") > -1) {
					System.out.print(" POS=" + atts.getValue("POS"));
				}
				System.out.println(" >");
			}

			depth++;
			if ((atts.getIndex("NAME") > -1) && atts.getValue("NAME").indexOf("_input_architecture") > -1) {
				howMany = 2;
			}
			if ((howMany > 0) || (!firstTime)) {
				if (localName.equals("file")) {
					// set the actual file we are in
					file = atts.getValue("NAME").replaceAll("&lt;", "<").replaceAll("&gt;", ">");
				} else if (localName.equals("path")) {
					// bi.setBenchItPath(atts.getValue("PATH"));
				} else if (bigdata
						&& !file.isEmpty()
						&& (localName.equals("integer") || localName.equals("boolean")
								|| localName.equals("none") || localName.equals("datetime")
								|| localName.equals("float") || localName.equals("list")
								|| localName.equals("multiple") || localName.equals("string")
								|| localName.equals("vector") || localName.equals("list")
								|| localName.equals("freetext") || localName.equals("restrictedtext"))) {
					// process data only inside <bigdata> and inside
					// a <file> section
					if (localName.equals("integer")) {
						type = BIGType.Integer;
					} else if (localName.equals("none")) {
						type = BIGType.None;
					} else if (localName.equals("boolean")) {
						type = BIGType.Boolean;
					} else if (localName.equals("float")) {
						type = BIGType.Float;
					} else if (localName.equals("list")) {
						type = BIGType.List;
					} else if (localName.equals("multiple")) {
						type = BIGType.Multiple;
					} else if (localName.equals("string")) {
						type = BIGType.String;
					} else if (localName.equals("list")) {
						type = BIGType.List;
					} else if (localName.equals("freetext")) {
						type = BIGType.FreeText;
					} else if (localName.equals("restrictedtext")) {
						type = BIGType.RestrictedText;
					} else {
						type = BIGType.None;
					}

					// try to get an existing version of this entry
					ent = bi.getEntry(file, atts.getValue("NAME"));
					// or create a new one
					if (ent == null) {
						ent = new BIGEntry(atts.getValue("NAME"));
						bi.addEntry(file, ent);
					}

					// do some action typeindependent

					// process POS
					try {
						i = Integer.parseInt(atts.getValue("POS"));
					} catch (NumberFormatException e) {
						String err = "BIGXMLParser: warning: unable to recognize \"" + atts.getValue("POS")
								+ "\" as integer - setting to 0\n" + "for value POS of intem= \""
								+ atts.getValue("NAME") + "\" " + "in File: " + locator.getSystemId() + " line "
								+ locator.getLineNumber();
						System.err.println(err);
						i = 0;
					}
					ent.setPos(i);

					// process MultipleCHOICECOUNT
					if (atts.getIndex("MultipleCHOICECOUNT") > -1) {
						try {
							i = Integer.parseInt(atts.getValue("MultipleCHOICECOUNT"));
						} catch (NumberFormatException e) {
							String err = "BIGXMLParser: warning: unable to recognize \""
									+ atts.getValue("MultipleCHOICECOUNT") + "\" as integer - setting to 0\n"
									+ "for value MultipleCHOICECOUNT of intem= \"" + atts.getValue("NAME") + "\" "
									+ "in File: " + locator.getSystemId() + " line " + locator.getLineNumber();
							System.err.println(err);
							i = 0;
						}
						ent.setMultipleChoiceCount(i);
					}

					// process ACTIVESTATUS
					if (atts.getValue("ACTIVESTATUS").equals("true")) {
						ent.setActiveStatus(true);
					} else if (atts.getValue("ACTIVESTATUS").equals("false")) {
						ent.setActiveStatus(false);
					} else {
						String err = "BIGXMLParser:  warning: unable to recognize \""
								+ atts.getValue("ACTIVESTATUS") + "\" as boolean - setting to \"false\""
								+ "in File: " + locator.getSystemId() + " line " + locator.getLineNumber();
						System.err.println(err);
						ent.setActiveStatus(false);
					}

					// process VIEWNAME
					ent.setViewName(atts.getValue("VIEWNAME"));

					// process TOOLTIPTEXT
					ent.setToolTipText(atts.getValue("TOOLTIPTEXT"));

					// process HELP
					ent.setHelp(atts.getValue("HELP"));

					// process ACTION
					String action = atts.getValue("ACTION");
					if (action != null && (action.isEmpty() || action.equals("null")))
						action = null;
					ent.setAction(action);

					// process PRIORITY
					DetailLevel det;
					String sDet = "???";
					try {
						sDet = atts.getValue("DETAILLEVEL");
						if (sDet == null || sDet.isEmpty()) {
							sDet = atts.getValue("PRIORITY");
							i = Integer.parseInt(sDet);
							det = DetailLevel.values()[i];
						} else
							det = DetailLevel.valueOf(sDet);
					} catch (Exception e) {
						String err = "BIGXMLParser: warning: unable to recognize \"" + sDet
								+ "\" as Detaillevel - setting to Low" + "in File: " + locator.getSystemId()
								+ " line " + locator.getLineNumber();
						System.err.println(err);
						det = DetailLevel.Low;
					}
					ent.setDetailLevel(det);

					// process NECESSITY
					if (atts.getValue("NECESSITY").equals("true")) {
						ent.setNecessity(true);
					} else if (atts.getValue("NECESSITY").equals("false")) {
						ent.setNecessity(false);
					} else {
						String err = "BIGXMLParser: warning: unable to recognize \""
								+ atts.getValue("NECESSITY") + "\" as boolean - setting to \"false\"" + "in File: "
								+ locator.getSystemId() + " line " + locator.getLineNumber();
						System.err.println(err);
						ent.setNecessity(false);
					}

					// do typedependent action
					switch (type) {

						case None :
							try {
								ent.setType(BIGType.None);
							} catch (BIGAccessViolationException e) {
								String err = "BIGXMLParser: error setting type for element \""
										+ atts.getValue("NAME") + "\" - type already set";
								System.err.println(err);
							}
							break;

						case Integer :
							// set type integer
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.Integer);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}

							// process DEFAULTVALUE integer
							try {
								in = new Integer(atts.getValue("DEFAULTVALUE"));
							} catch (NumberFormatException e) {
								String err = "BIGXMLParser: warning: unable to recognize \""
										+ atts.getValue("DEFAULTVALUE") + "\" as integer - setting to 0" + "in File: "
										+ locator.getSystemId() + " line " + locator.getLineNumber();
								System.err.println(err);
								in = new Integer(0);
							}
							ent.setDefaultValue(in);

							break;

						case Boolean :
							// set type boolean
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.Boolean);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}

							// process DEFAULTVALUE boolean
							if (atts.getValue("DEFAULTVALUE").equals("true")) {
								b = new Boolean(true);
							} else if (atts.getValue("DEFAULTVALUE").equals("false")) {
								b = new Boolean(false);
							} else {
								String err = "BIGXMLParser: unable to recognize \"" + atts.getValue("DEFAULTVALUE")
										+ "\" as boolean - setting to false" + "in File: " + locator.getSystemId()
										+ " line " + locator.getLineNumber();
								System.err.println(err);
								b = new Boolean(false);
							}
							ent.setDefaultValue(b);

							break;

						case Float :
							// set type float
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.Float);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}

							// process DEFAULTVALUE float
							try {
								d = new Double(atts.getValue("DEFAULTVALUE"));
							} catch (NumberFormatException e) {
								String err = "BIGXMLParser: unable to recognize \"" + atts.getValue("DEFAULTVALUE")
										+ "\" as double - setting to 0" + "in File: " + locator.getSystemId()
										+ " line " + locator.getLineNumber() + "\n";

								System.err.println(err);
								d = new Double(0);
							}
							ent.setDefaultValue(d);

							break;

						// process List
						case List :

							// set type list
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.List);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}
							// process DEFAULTVALUE list
							bs1 = new BIGStrings();
							bs1.setCommaText(atts.getValue("DEFAULTVALUE").replaceAll("&quot;", "\""));
							ent.setDefaultValue(bs1);

							// process MultipleCHOICE list
							bs2 = new BIGStrings();

							bs2.setCommaText(atts.getValue("UNSELECTED").replaceAll("&quot;", "\""));
							bs2 = bs2.concat(bs1);
							ent.setMultipleChoice(bs2);
							break;

						case Multiple :
							// set type string
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.Multiple);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}

							// process DEFAULTVALUE multiple
							try {
								iMultipleD = Integer.parseInt(atts.getValue("DEFAULTVALUE"));
							} catch (NumberFormatException e) {
								String err = "BIGXMLParser: warning: unable to recognize \""
										+ atts.getValue("DEFAULTVALUE") + "\" as integer - setting to -2" + "in File: "
										+ locator.getSystemId() + " line " + locator.getLineNumber();
								System.err.println(err);
								iMultipleD = -2;
							}

							// prepare the entry for the parts of this multiple
							ent.setMultipleChoice(new BIGStrings());
							// ent.setMultipleChoiceCount(0);

							// remember the entry for the parts
							eMultiple = ent;
							break;

						case String :
							// set type string
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.String);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}

							// process DEFAULTVALUE string
							ent.setDefaultValue(atts.getValue("DEFAULTVALUE"));
							break;

						case FreeText :
							// set type string
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.FreeText);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}

							// process DEFAULTVALUE string
							ent.setDefaultValue(atts.getValue("DEFAULTVALUE"));
							break;

						case RestrictedText :
							// set type string
							if (ent.getType() == BIGType.None) {
								try {
									ent.setType(BIGType.RestrictedText);
								} catch (BIGAccessViolationException e) {
									String err = "BIGXMLParser: error setting type for element \""
											+ atts.getValue("NAME") + "\" - type already set";
									System.err.println(err);
								}
							}

					}

				} else if (localName.equals("multiple_part") && eMultiple != null) {
					// get id of actual part
					try {
						i = Integer.parseInt(atts.getValue("ID"));
					} catch (NumberFormatException e) {
						String err = "BIGXMLParser: warning: unable to recognize \"" + atts.getValue("ID")
								+ "\" as integer - setting to -3\n" + "in File: " + locator.getSystemId()
								+ " line " + locator.getLineNumber();
						System.err.println(err);
						i = -3;
					}

					// if this is the DEFAULTVALUE (multiple)
					if (iMultipleD == i) {
						eMultiple.setDefaultValue(atts.getValue("VALUE"));
						iMultipleD = -1;
					}

					// add this part to the list of possible choices
					eMultiple.getMultipleChoice().add(atts.getValue("VALUE"));
					// increase the number of possible choices by 1
					// eMultiple.setMultipleChoiceCount(eMultiple.getMultipleChoiceCount() + 1);

				} else if (localName.equals("bigdata")) {
					bigdata = true;
				} else if (localName.equals("writtenAt")) {} else {
					System.err.println("BIGXMLParser: " + locator.getLineNumber()
							+ " no action defined for this tag: \"" + localName + "\"");
				}

			}
		}

		/**
		 * method used by SAX the parser tells us, that it has found a element end </...>
		 */
		public void endElement(String namespaceURI, String localName, String rawName)
				throws SAXException {

			depth--;
			if (debug) {
				System.out.println("BIGXMLParser: " + tabs(depth) + "</" + localName + ">");
			}
			// leave the bigdatasection
			if (localName.equals("bigdata")) {
				bigdata = false;
			} else if (localName.equals("file")) {
				file = new String();
			} else if (localName.equals("multiple")) {
				// if we have not set this to -1 we have not found a defaultvalue
				if (iMultipleD > -1) {
					String err = "BIGXMLParser: warning: no Defaultvalue set for \"" + eMultiple.getName()
							+ "\" in File: " + locator.getSystemId() + " line " + locator.getLineNumber();
					System.err.println(err);
				}
				// delete this reference, we do not need this anymore, we have left this <multiple> section
				eMultiple = null;
			}
		}

		/**
		 * method used by SAX the parser tells us, that it has found characters (inside <element></element>)
		 */
		public void characters(char[] ch, int start, int end) throws SAXException {}

		/**
		 * method used by SAX the parser tells us, that it has found ignorable whitespace
		 * org.apache.xml.crimson.parser.XMLReaderImpl does not use this
		 */
		public void ignorableWhitespace(char[] ch, int start, int end) throws SAXException {}

		/**
		 * method used by SAX the parser tells us, that it has skipped entities
		 */
		public void skippedEntity(String name) throws SAXException {}

	}

	private class BIGObservable extends Observable {
		private int oldValue = -1;

		public void setProgress(int value) {
			if (oldValue != value) {
				oldValue = value;
				setChanged();
				notifyObservers(new Integer(value));
				if (debug) {
					System.out.println("BIGXMLParser: writing file [" + current + "]");
				}
			}
		}
	}

	private class BIGWriteThread extends Thread {
		@Override
		public void run() {
			if (debug) {
				System.out.println("BIGXMLParser: Writing File: " + filename);
			}

			try {
				Thread.sleep(1);
			} catch (InterruptedException e1) {}

			current = 0;
			min = 0;
			max = bi.getAllEntriesCount();
			difference = 100d / max;

			if (debug) {
				System.out
						.println("BIGXMLParser: min=" + min + " max=" + max + " difference=" + difference);
			}

			// a String containig temporary data
			String tempStr;
			String tempStr1;

			// a BIGStrings containing all filenames
			BIGStrings files;
			// an iterator over the content of the current file
			Iterator<BIGEntry> contentIt;

			// adding an XML header to our Sring (file)
			// this is a very simple header without encoding or standalone
			out.add("<?xml version=\"1.0\"?>");

			// data section (open root element)
			out.add("<bigdata>");

			// get actual date and add it to the file
			Calendar rightNow = Calendar.getInstance();
			out.add("\t<writtenAt value=\"" + rightNow.getTime() + "\"></writtenAt>");

			// add BenchItProjectPath
			out.add("\t<path PATH=\"" + bi.getBenchItPath() + "\"></path>");

			// get all filenames and
			files = bi.getAllFilenames();
			Iterator<String> fileIt = files.iterator();
			while (fileIt.hasNext()) {
				// tempStr contains <hostname>
				tempStr = fileIt.next();

				// tempStr1 contains #lt;hostname#gt;
				tempStr1 = tempStr.replaceAll("<", "&lt;").replaceAll(">", "&gt;");

				// get an iterator over all elements of this file section
				contentIt = bi.iterator(tempStr);

				// write <file name="blah"> or similar start tag for file section
				out.add("\t<file NAME=\"" + tempStr1 + "\">");

				// processing content of file section
				helper(contentIt);

				// write </file> or similar end tag for file section
				out.add("\t</file>");

				try {
					Thread.sleep(1);
				} catch (InterruptedException e2) {}
			}
			// finishing XML - close root element
			out.add("</bigdata>");

			// try {
			// FileWriter fw = new FileWriter(filename);
			// fw.write(out);
			// fw.close();
			out.saveToFile(filename);
			// } catch (IOException e) {
			// bi.getConsole().postMessage("Error writing file:\n"+e.getMessage(),gui.BIGConsole.ERROR,true);
			// }
			progress.setProgress(100);
		}
	}
}
/*****************************************************************************
 * Log-History
 *****************************************************************************/
