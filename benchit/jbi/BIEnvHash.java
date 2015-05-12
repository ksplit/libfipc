/********************************************************************
 * BenchIT - Performance Measurement for Scientific Applications
 * Contact: developer@benchit.org
 *
 * $Id: BIEnvHash.template.java 1 2009-09-11 12:26:19Z william $
 * $URL: svn+ssh://william@rupert.zih.tu-dresden.de/svn-base/benchit-root/BenchITv6/tools/BIEnvHash.template.java $
 * For license details see COPYING in the package base directory
 *******************************************************************/
import java.util.HashMap;
import java.util.Iterator;
import java.io.*;

public class BIEnvHash
{
   private static BIEnvHash instance = null;
   private HashMap table = null;
   private BIEnvHash()
   {
      bi_initTable();
      bi_fillTable();
   }
   public static BIEnvHash getInstance()
   {
      if ( instance == null ) instance = new BIEnvHash();
      return instance;
   }
   /** Puts a new key value mapping into the map and returns the
     * previously assigned value to the key, or null, if it's a
     * new mapping. */
   public String bi_setEnv( String key, String value )
   {
      return bi_put( key, value );
   }
   /** Puts a new key value mapping into the map and returns the
     * previously assigned value to the key, or null, if it's a
     * new mapping. */
   public String bi_put( String key, String value )
   {
      if ( table == null ) bi_initTable();
      Object ret = table.put( key, value );
      if ( ret == null ) return null;
      return (String)ret;
   }
   public String bi_getEnv( String key )
   {
      if (System.getenv(key)!=null) return System.getenv(key);
      return bi_get( key );
   }
   public String bi_get( String key )
   {
      if ( table == null ) return null;
      return (String)(table.get( key ));
   }
   public void bi_dumpTable()
   {
      if ( table == null ) return;
      printf( "\nHashtable dump of all known environment variables at compiletime:" );
      printf( "\n Key            | Value" );
      printf( "\n----------------------------------" );
      Iterator it = table.keySet().iterator();
      while ( it.hasNext() )
      {
         String key = (String)(it.next());
         String value = (String)(table.get( key ));
         printf( "\n " + key + " | " + value );
      }
      printf( "\n" + bi_size() + " entries in total.\n" );
      flush();
   }
   public void dumpToOutputBuffer( StringBuffer outputBuffer )
   {
      outputBuffer.append( "beginofenvironmentvariables\n" );
      Iterator it = table.keySet().iterator();
      while ( it.hasNext() )
      {
         String key = (String)(it.next());
         String value = (String)(table.get( key ));
         StringBuffer b = new StringBuffer();
         for ( int i = 0; i < value.length(); i++ )
         {
            if ( value.charAt( i ) == '"' ) {
               /* check equal number of escape chars->insert escape char */
               int cnt = 0;
               for ( int j = i - 1;  j >= 0; j-- ) {
                  if ( value.charAt( j ) == '\\' ) cnt++;
               }
               /* escape quote */
               if ( cnt % 2 == 0 ) {
                  b.append( '\\' );
               }
            }
            b.append( value.charAt( i ) );
         }
         value = b.toString();
         outputBuffer.append( key + "=\"" + value + "\"\n" );
      }
      outputBuffer.append( "endofenvironmentvariables\n" );
   }
   public int bi_size()
   {
      if ( table != null ) return table.size();
      return -1;
   }
   public void bi_initTable()
   {
      table = new HashMap( 1009 );
   }
   private void printf( String str )
   {
      System.out.print( str );
   }
   private void flush()
   {
      System.out.flush();
      System.err.flush();
   }
   /** Takes a PARAMETER file as argument and adds it's variables
     * to the environment HashMap. */
   public boolean bi_readParameterFile( String fileName )
   {
      boolean retval = false;
      File textFile = null;
      try {
         textFile = new File( fileName );
      }
      catch ( NullPointerException npe ) {
         return retval;
      }
      if ( textFile != null ) {
         if ( textFile.exists() ) {
            FileReader in;
            char ch = '\0';
            int size = (int)(textFile.length());
            int read = 0;
            char inData[] = new char[size + 1];
            try {
               in = new FileReader( textFile );
               in.read( inData, 0, size );
               in.close();
            }
            catch( IOException ex ) {
               return retval;
            } // scan inData for environment variables
            int scanned = 0;
            int it = 0;
            ch = inData[scanned];
            StringBuffer buffer = null;
            while ( scanned < size ) {
               buffer = new StringBuffer();
               // read a line
               while ( ch != '\n' ) {
                  if ( ch != '\r' ) {
                     buffer.append( ch );
                  }
                  scanned += 1;
                  ch = inData[scanned];
               }
               // trim leading and trailing white spaces
               String lineValue = buffer.toString().trim();
               // now check the line
               int eqIndex = -1;
               try {
                  eqIndex = lineValue.indexOf( "=" );
               }
               catch ( NullPointerException npe ) {
                  return retval;
               }
               // only check lines containing an equals sign, no $ sign,
               // and the start with a capital letter
               if ( eqIndex >= 0 && lineValue.indexOf( "$" ) < 0 &&
                  lineValue.substring( 0, 1 ).matches( "[A-Z]" ) ) {
                  String varname = null;
                  String varvalue = null;
                  try {
                     varname = lineValue.substring( 0 , eqIndex );
                     varvalue = lineValue.substring( eqIndex + 1 );
                  }
                  catch ( IndexOutOfBoundsException ioobe ) {
                  }
                  if ( ( varname != null ) && ( varvalue != null ) ) {
                     boolean st1=false, en1=false, st2=false, en2=false;
                     boolean st3=false, en3=false;
                     try {
                        st1=varvalue.startsWith( "'" );
                        en1=varvalue.endsWith( "'" );
                        st2=varvalue.startsWith( "(" );
                        en2=varvalue.endsWith( ")" );
                        st3=varvalue.startsWith( "\"" );
                        en3=varvalue.endsWith( "\"" );
                     }
                     catch ( NullPointerException npe ) {
                        return retval;
                     }
                     if ( ( ( st1 == en1 ) && ( ( st1 != st2 ) && ( st1!=st3 ) ) ) ||
                          ( ( st2 == en2 ) && ( ( st2 != st1 ) && ( st2!=st3 ) ) ) ||
                          ( ( st3 == en3 ) && ( ( st3 != st1 ) && ( st3!=st2 ) ) ) ||
                          ( ( st1 == en1 ) && ( st2 == en2 ) && ( st3 == en3 )
                          && ( st1 == st2 ) && ( st2 == st3 ) && ( st3 == false ) ) ) {
                        table.put( varname, varvalue );
                     }
                  }
               }
               scanned += 1;
               ch = inData[scanned];
               it++;
            }
         }
         else {
            System.err.println( "BenchIT: PARAMETER file \"" + fileName
               + "\" doesn't exist." );
            return retval;
         }
      }
      else {
         System.err.println( "BenchIT: PARAMETER file \"" + fileName
               + "\" not found." );
         return retval;
      }
      return true;
   }
   /* special case: generated from outside and code will be
      appended to this file */
   /** Fills the table with predefined content. */
   public void bi_fillTable()
   {

      bi_put("BENCHITROOT", "/users/sbauer/memtest2/benchit");
      bi_put("BENCHIT_ARCH_SHORT", "InX4");
      bi_put("BENCHIT_ARCH_SPEED", "2393M");
      bi_put("BENCHIT_CC", "gcc");
      bi_put("BENCHIT_CC_C_FLAGS", "");
      bi_put("BENCHIT_CC_C_FLAGS_HIGH", "-O0");
      bi_put("BENCHIT_CC_C_FLAGS_OMP", "");
      bi_put("BENCHIT_CC_C_FLAGS_STD", "-O2");
      bi_put("BENCHIT_CC_LD", "gcc");
      bi_put("BENCHIT_CC_L_FLAGS", "-lm");
      bi_put("BENCHIT_COMMENT", "memory read latency");
      bi_put("BENCHIT_COMPILETIME_CC", "gcc");
      bi_put("BENCHIT_COMPILE_SH_IS_CALLED", "1");
      bi_put("BENCHIT_CPP_ACML", "");
      bi_put("BENCHIT_CPP_ATLAS", "");
      bi_put("BENCHIT_CPP_BLAS", "");
      bi_put("BENCHIT_CPP_ESSL", "");
      bi_put("BENCHIT_CPP_FFTW3", "");
      bi_put("BENCHIT_CPP_MKL", "");
      bi_put("BENCHIT_CPP_MPI", " -DUSE_MPI");
      bi_put("BENCHIT_CPP_PAPI", "-DUSE_PAPI");
      bi_put("BENCHIT_CPP_PCL", " -DUSE_PCL");
      bi_put("BENCHIT_CPP_PTHREADS", "");
      bi_put("BENCHIT_CPP_PVM", "");
      bi_put("BENCHIT_CPP_SCSL", "");
      bi_put("BENCHIT_CROSSCOMPILE", "0");
      bi_put("BENCHIT_CXX", "g++");
      bi_put("BENCHIT_CXX_C_FLAGS", "");
      bi_put("BENCHIT_CXX_C_FLAGS_HIGH", "-O3");
      bi_put("BENCHIT_CXX_C_FLAGS_OMP", "");
      bi_put("BENCHIT_CXX_C_FLAGS_STD", "-O2");
      bi_put("BENCHIT_CXX_LD", "g++");
      bi_put("BENCHIT_CXX_L_FLAGS", "-lm");
      bi_put("BENCHIT_DEBUGLEVEL", "0");
      bi_put("BENCHIT_DEFINES", " -DDEBUGLEVEL=0");
      bi_put("BENCHIT_ENVIRONMENT", "DEFAULT");
      bi_put("BENCHIT_F77", "");
      bi_put("BENCHIT_F77_C_FLAGS", "");
      bi_put("BENCHIT_F77_C_FLAGS_HIGH", "");
      bi_put("BENCHIT_F77_C_FLAGS_OMP", "");
      bi_put("BENCHIT_F77_C_FLAGS_STD", "");
      bi_put("BENCHIT_F77_LD", "");
      bi_put("BENCHIT_F77_L_FLAGS", "-lm");
      bi_put("BENCHIT_F90", "");
      bi_put("BENCHIT_F90_C_FLAGS", "");
      bi_put("BENCHIT_F90_C_FLAGS_HIGH", "");
      bi_put("BENCHIT_F90_C_FLAGS_OMP", "");
      bi_put("BENCHIT_F90_C_FLAGS_STD", "");
      bi_put("BENCHIT_F90_LD", "");
      bi_put("BENCHIT_F90_L_FLAGS", "-lm");
      bi_put("BENCHIT_F90_SOURCE_FORMAT_FLAG", "");
      bi_put("BENCHIT_F95", "");
      bi_put("BENCHIT_F95_C_FLAGS", "");
      bi_put("BENCHIT_F95_C_FLAGS_HIGH", "");
      bi_put("BENCHIT_F95_C_FLAGS_OMP", "");
      bi_put("BENCHIT_F95_C_FLAGS_STD", "");
      bi_put("BENCHIT_F95_LD", "");
      bi_put("BENCHIT_F95_L_FLAGS", "-lm");
      bi_put("BENCHIT_F95_SOURCE_FORMAT_FLAG", "");
      bi_put("BENCHIT_FILENAME_COMMENT", "0");
      bi_put("BENCHIT_HOSTNAME", "pc405.emulab.net");
      bi_put("BENCHIT_IGNORE_PARAMETER_FILE", "0");
      bi_put("BENCHIT_INCLUDES", "-I. -I/users/sbauer/memtest2/benchit -I/users/sbauer/memtest2/benchit/tools");
      bi_put("BENCHIT_INTERACTIVE", "0");
      bi_put("BENCHIT_JAVA", "");
      bi_put("BENCHIT_JAVAC", "");
      bi_put("BENCHIT_JAVAC_FLAGS", "");
      bi_put("BENCHIT_JAVAC_FLAGS_HIGH", "");
      bi_put("BENCHIT_JAVA_FLAGS", "");
      bi_put("BENCHIT_JAVA_HOME", "");
      bi_put("BENCHIT_KERNELBINARY", "/users/sbauer/memtest2/benchit/bin/arch_x86_64.memory_latency.C.pthread.0.read.0");
      bi_put("BENCHIT_KERNELBINARY_ARGS", " ");
      bi_put("BENCHIT_KERNELNAME", "arch_x86_64.memory_latency.C.pthread.0.read");
      bi_put("BENCHIT_KERNEL_ACCESSES", "1024");
      bi_put("BENCHIT_KERNEL_ALIGNMENT", "256");
      bi_put("BENCHIT_KERNEL_ALLOC", "L");
      bi_put("BENCHIT_KERNEL_COMMENT", " 0B missaligned, alloc: L, hugep.: 0, use: M4, flush: 111 - M2, TLB: 0");
      bi_put("BENCHIT_KERNEL_COUNTERS", "PAPI_L2_DCM");
      bi_put("BENCHIT_KERNEL_CPU_LIST", "0,1,4");
      bi_put("BENCHIT_KERNEL_ENABLE_PAPI", "0");
      bi_put("BENCHIT_KERNEL_FLUSH_ACCESSES", "2");
      bi_put("BENCHIT_KERNEL_FLUSH_L1", "1");
      bi_put("BENCHIT_KERNEL_FLUSH_L2", "1");
      bi_put("BENCHIT_KERNEL_FLUSH_L3", "1");
      bi_put("BENCHIT_KERNEL_FLUSH_MODE", "M");
      bi_put("BENCHIT_KERNEL_HUGEPAGES", "0");
      bi_put("BENCHIT_KERNEL_HUGEPAGE_DIR", "/mnt/huge");
      bi_put("BENCHIT_KERNEL_MAX", "25165824");
      bi_put("BENCHIT_KERNEL_MIN", "16384");
      bi_put("BENCHIT_KERNEL_OFFSET", "0");
      bi_put("BENCHIT_KERNEL_RUNS", "4");
      bi_put("BENCHIT_KERNEL_SERIALIZATION", "mfence");
      bi_put("BENCHIT_KERNEL_SHARE_CPU", "7");
      bi_put("BENCHIT_KERNEL_STEPS", "100");
      bi_put("BENCHIT_KERNEL_TIMEOUT", "3600");
      bi_put("BENCHIT_KERNEL_TLB_MODE", "0");
      bi_put("BENCHIT_KERNEL_USE_ACCESSES", "4");
      bi_put("BENCHIT_KERNEL_USE_MODE", "M");
      bi_put("BENCHIT_LD_LIBRARY_PATH", "/users/sbauer/memtest2/benchit/jbi/jni");
      bi_put("BENCHIT_LIB_ACML", " -lacml");
      bi_put("BENCHIT_LIB_ATLAS", " -latlas");
      bi_put("BENCHIT_LIB_BLAS", "-lblas");
      bi_put("BENCHIT_LIB_ESSL", " -lessl");
      bi_put("BENCHIT_LIB_FFTW3", " -lfftw3");
      bi_put("BENCHIT_LIB_MKL", " -lmkl");
      bi_put("BENCHIT_LIB_MPI", "");
      bi_put("BENCHIT_LIB_PAPI", "");
      bi_put("BENCHIT_LIB_PCL", "");
      bi_put("BENCHIT_LIB_PTHREAD", "");
      bi_put("BENCHIT_LIB_PVM", "");
      bi_put("BENCHIT_LIB_SCSL", " -lscsl");
      bi_put("BENCHIT_LOCAL_CC", "gcc");
      bi_put("BENCHIT_LOCAL_CC_C_FLAGS", "");
      bi_put("BENCHIT_LOCAL_CC_L_FLAGS", "-lm");
      bi_put("BENCHIT_MANDATORY_FILES", "benchit.c interface.h tools/envhashbuilder.c tools/bienvhash.template.c tools/bienvhash.h tools/stringlib.c tools/stringlib.h tools/bitWriter.c tools/bitWriter.h tools/gnuWriter.c tools/gnuWriter.h tools/output.c tools/output.h ");
      bi_put("BENCHIT_MPICC", "gcc");
      bi_put("BENCHIT_MPICC_C_FLAGS", "");
      bi_put("BENCHIT_MPICC_C_FLAGS_HIGH", "-O3");
      bi_put("BENCHIT_MPICC_C_FLAGS_OMP", "");
      bi_put("BENCHIT_MPICC_C_FLAGS_STD", "-O2");
      bi_put("BENCHIT_MPICC_LD", "gcc");
      bi_put("BENCHIT_MPICC_L_FLAGS", "-lm -lmpi");
      bi_put("BENCHIT_MPICXX_C_FLAGS", "");
      bi_put("BENCHIT_MPICXX_C_FLAGS_HIGH", "-O3");
      bi_put("BENCHIT_MPICXX_C_FLAGS_OMP", "");
      bi_put("BENCHIT_MPICXX_C_FLAGS_STD", "-O2");
      bi_put("BENCHIT_MPICXX_LD", "gcc");
      bi_put("BENCHIT_MPICXX_L_FLAGS", "-lm -lmpi");
      bi_put("BENCHIT_MPIF77", "");
      bi_put("BENCHIT_MPIF77_C_FLAGS", "");
      bi_put("BENCHIT_MPIF77_C_FLAGS_HIGH", "");
      bi_put("BENCHIT_MPIF77_C_FLAGS_OMP", "");
      bi_put("BENCHIT_MPIF77_C_FLAGS_STD", "");
      bi_put("BENCHIT_MPIF77_LD", "");
      bi_put("BENCHIT_MPIF77_L_FLAGS", "");
      bi_put("BENCHIT_MPIRUN", "mpirun");
      bi_put("BENCHIT_MPIXX", "g++");
      bi_put("BENCHIT_NODENAME", "node0");
      bi_put("BENCHIT_NUM_CPUS", "8");
      bi_put("BENCHIT_NUM_PROCESSES", "");
      bi_put("BENCHIT_NUM_THREADS_PER_PROCESS", "");
      bi_put("BENCHIT_OPTIONAL_FILES", "LOCALDEFS/PROTOTYPE_input_architecture LOCALDEFS/PROTOTYPE_input_display ");
      bi_put("BENCHIT_PARAMETER_FILE", "/users/sbauer/memtest2/benchit/kernel/arch_x86_64/memory_latency/C/pthread/0/read/PARAMETERS");
      bi_put("BENCHIT_PROGRESS_DIR", "progress");
      bi_put("BENCHIT_RUN_ACCURACY", "2");
      bi_put("BENCHIT_RUN_COREDUMPLIMIT", "0");
      bi_put("BENCHIT_RUN_EMAIL_ADDRESS", "");
      bi_put("BENCHIT_RUN_LINEAR", "0");
      bi_put("BENCHIT_RUN_MAX_MEMORY", "10094");
      bi_put("BENCHIT_RUN_OUTPUT_DIR", "/users/sbauer/memtest2/benchit/output");
      bi_put("BENCHIT_RUN_QUEUENAME", "");
      bi_put("BENCHIT_RUN_REDIRECT_CONSOLE", "");
      bi_put("BENCHIT_RUN_TEST", "0");
      bi_put("BENCHIT_RUN_TIMELIMIT", "3600");
      bi_put("BENCHIT_USE_VAMPIR_TRACE", "0");
      bi_put("BR", "0");
      bi_put("COMMENT", "");
      bi_put("COMPILE_GLOBAL", "1");
      bi_put("CONFIGURE_MODE", "COMPILE");
      bi_put("CURDIR", "/users/sbauer/memtest2/benchit");
      bi_put("DEST", "/users/sbauer/memtest2/benchit/bin/");
      bi_put("DISPLAY", "localhost:10.0");
      bi_put("HLL", "C");
      bi_put("HOME", "/users/sbauer");
      bi_put("IFS", "' 	");
      bi_put("KERNELBASEDIR", "/users/sbauer/memtest2/benchit/kernel");
      bi_put("KERNELDIR", "/users/sbauer/memtest2/benchit/kernel/arch_x86_64/memory_latency/C/pthread/0/read");
      bi_put("KERNELNAME_FULL", "");
      bi_put("LOCAL_BENCHITC_COMPILER", "gcc  -O2  -DDEBUGLEVEL=0");
      bi_put("LOCAL_BENCHIT_COMPILER", "C");
      bi_put("LOCAL_BENCHIT_COMPILERFLAGS", "");
      bi_put("LOCAL_FILES_TO_COPY", "");
      bi_put("LOCAL_FILES_TO_DELETE", "");
      bi_put("LOCAL_KERNEL_COMPILER", "gcc");
      bi_put("LOCAL_KERNEL_COMPILERFLAGS", " -g  -O0 -I. -I/users/sbauer/memtest2/benchit -I/users/sbauer/memtest2/benchit/tools -I/users/sbauer/memtest2/benchit/tools/hw_detect");
      bi_put("LOCAL_KERNEL_FILES", "");
      bi_put("LOCAL_LINKER", "");
      bi_put("LOCAL_LINKERFLAGS", "-lm ");
      bi_put("LOCAL_MODULE_FILES", "");
      bi_put("LOGNAME", "root");
      bi_put("LastChecked", "");
      bi_put("MAIL", "/var/mail/root");
      bi_put("OLDCWD", "/users/sbauer/memtest2/benchit");
      bi_put("OLDIR", "/users/sbauer/memtest2/benchit");
      bi_put("OLDPWD", "/users/sbauer/memtest2/benchit");
      bi_put("OMP_DYNAMIC", "FALSE");
      bi_put("OMP_NESTED", "FALSE");
      bi_put("OMP_NUM_THREADS", "1");
      bi_put("OPTIND", "1");
      bi_put("PATH", "/users/sbauer/memtest2/benchit/tools:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin");
      bi_put("PPID", "7562");
      bi_put("PS1", "# ");
      bi_put("PS2", "> ");
      bi_put("PS4", "+ ");
      bi_put("PWD", "/users/sbauer/memtest2/benchit/tools");
      bi_put("SCRIPTNAME", "COMPILE.SH");
      bi_put("SHELL", "/bin/tcsh");
      bi_put("SHELLSCRIPT_DEBUG", "0");
      bi_put("SUDO_COMMAND", "./GUI.sh");
      bi_put("SUDO_GID", "7394");
      bi_put("SUDO_UID", "13844");
      bi_put("SUDO_USER", "sbauer");
      bi_put("TERM", "screen");
      bi_put("USEJAVA", "0");
      bi_put("USER", "root");
      bi_put("USERNAME", "root");
      bi_put("_CMDLINE_VARLIST", "BENCHIT_KERNELBINARY BENCHIT_KERNELBINARY_ARGS BENCHIT_CMDLINE_ARG_FILENAME_COMMENT BENCHIT_CMDLINE_ARG_PARAMETER_FILE BENCHIT_CMDLINE_ARG_IGNORE_PARAMETER_FILE BENCHIT_NODENAME BENCHIT_CROSSCOMPILE BENCHIT_CMDLINE_ARG_NUM_CPUS BENCHIT_CMDLINE_ARG_NUM_PROCESSES BENCHIT_CMDLINE_ARG_NUM_THREADS_PER_PROCESS BENCHIT_CMDLINE_ARG_RUN_CLEAN BENCHIT_CMDLINE_ARG_RUN_COREDUMPLIMIT BENCHIT_CMDLINE_ARG_RUN_EMAIL_ADDRESS BENCHIT_CMDLINE_ARG_RUN_MAX_MEMORY BENCHIT_CMDLINE_ARG_RUN_QUEUENAME BENCHIT_CMDLINE_ARG_RUN_QUEUETIMELIMIT BENCHIT_CMDLINE_ARG_RUN_REDIRECT_CONSOLE BENCHIT_CMDLINE_ARG_RUN_TEST BENCHIT_CMDLINE_ARG_RUN_USE_MPI BENCHIT_CMDLINE_ARG_RUN_USE_OPENMP ");
      bi_put("_VARLIST", "'BENCHITROOT");
      bi_put("myfile", "tools/output.h");
      bi_put("myval", "");
      bi_put("myvar", "BENCHIT_CMDLINE_ARG_RUN_USE_OPENMP");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BLAS", "NO REVISION, UNABLE TO READ (Sat Jan 18 16:00:36 2014)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_LOC_REPL", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_REPEAT_H", "NO REVISION (Mon Sep  2 07:18:08 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_OUTPUT_H", "NO REVISION (Wed Jul 10 10:38:47 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_CONFIGURE", "NO REVISION (Tue Jul 16 09:31:45 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BIENVHASH_TEMPLATE_JAVA", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_ENVHASHBUILDER_C", "NO REVISION (Mon Jun 17 06:57:27 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_CHANGE_SH_SH", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_FEATURES", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BMERGE_SH", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_COMPILERVERSION", "NO REVISION, UNABLE TO READ (Sat Jan 18 16:00:39 2014)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BITWRITER_C", "NO REVISION (Fri Mar  6 13:54:33 2015)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_ERROR_H", "NO REVISION (Wed Aug 25 04:53:18 2010)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_QUICKVIEW_SH", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_GNUWRITER_C", "NO REVISION (Mon Aug 12 10:58:00 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BIENVHASH_TEMPLATE_C", "NO REVISION (Wed Jul 10 10:38:47 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_STRINGLIB_H", "NO REVISION (Wed Aug 25 04:53:18 2010)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_CBLAS", "NO REVISION, UNABLE TO READ (Sat Jan 18 16:00:36 2014)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BENCHSCRIPT_C", "NO REVISION (Mon Jun 17 06:57:27 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_ENVHASHBUILDER", "NO REVISION (Fri Apr  3 16:37:39 2015)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_FILEVERSION_C", "*/ (Mon Jun 17 06:57:27 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BITWRITER_H", "NO REVISION (Tue Jul  2 09:32:22 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_LOC_CONVERT_SH", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_REFERENCE_RUN_SH", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_ALIGNED_MEMORY_H", "NO REVISION (Wed Aug 25 04:53:40 2010)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_OUTPUT_C", "NO REVISION (Tue Jul  2 10:05:03 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_FILEVERSION", "NO REVISION (Fri Apr  3 16:37:39 2015)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_HELPER_SH", "1.14 (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_FIRSTTIME", "NO REVISION (Wed Aug 25 04:53:18 2010)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_TMP_ENV", "NO REVISION (Fri Apr  3 16:37:39 2015)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BENCHSCRIPT_H", "NO REVISION (Wed Aug 25 04:53:40 2010)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_ENVIRONMENTS", "NO REVISION, UNABLE TO READ (Sat Jan 18 16:00:39 2014)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_IRODS", "NO REVISION, UNABLE TO READ (Sat Jan 18 16:00:39 2014)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_STRINGLIB_C", "NO REVISION (Thu Aug  8 06:25:10 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_HW_DETECT", "NO REVISION, UNABLE TO READ (Thu Mar  5 11:03:55 2015)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_GNUWRITER_H", "NO REVISION (Wed Jul 10 10:38:47 2013)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_CMDLINEPARAMS", "NO REVISION (Thu Nov 12 06:13:49 2009)");
      bi_put("BENCHIT_KERNEL_FILE_VERSION_BIENVHASH_H", "NO REVISION (Wed Jul 10 10:38:47 2013)");
   }
}
