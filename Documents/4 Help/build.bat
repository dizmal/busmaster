rem @echo off

if exist "%ProgramFiles(x86)%" (
	set PF=%ProgramFiles(x86)%
) else (
	if exist "%ProgramFiles%" (
		set PF=%ProgramFiles%
	) else (
		echo Program files not set. Build failed!
		goto END
	)
)


:JAVA_FIND
for /d %%i in ("%PF%\Java\jdk*") do set JAVA_HOME=%%i
if exist "%JAVA_HOME%\lib\tools.jar" goto SERNA_FIND

:JAVA_NOT_FOUND
echo Java Development Kit not found. Build failed!
goto END

:SERNA_FIND
for /d %%i in ("%PF%\Syntext\Serna Free*") do set SERNA_HOME=%%i
if exist "%SERNA_HOME%" goto DITA_FIND

:SERNA_NOT_FOUND
echo Serna not found. Build failed!
goto END

:DITA_FIND
for /d %%i in ("%SERNA_HOME%\plugins\dita\DITA-OT1.5*") do set DITA_HOME=%%i
if exist "%DITA_HOME%" goto DITA_1_5
for /d %%i in ("%SERNA_HOME%\plugins\dita\DITA-OT1.4*") do set DITA_HOME=%%i
if exist "%DITA_HOME%" goto DITA_1_4

:DITA_NOT_FOUND
echo DITA not found. Build failed!
goto END

:DITA_1_4
echo Please consider an upgrade to the recommended DITA version 1.5 or higher
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\dost.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\resolver.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\fop.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\avalon-framework-cvs-20020806.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\batik.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\xalan.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\xercesImpl.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\xml-apis.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\icu4j.jar
set ANT_OPTS=-Xmx512m %ANT_OPTS%
goto HTMLHELP_FIND

:DITA_1_5
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\dost.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\commons-codec-1.4.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\resolver.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\icu4j.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-dom.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-dom4j.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-jdom.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-s9api.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-sql.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-xom.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-xpath.jar
set CLASSPATH=%CLASSPATH%;%DITA_HOME%\lib\saxon\saxon9-xqj.jar
set ANT_OPTS=-Xmx512m %ANT_OPTS%
set ANT_OPTS=%ANT_OPTS% -Djavax.xml.transform.TransformerFactory=net.sf.saxon.TransformerFactoryImpl
goto HTMLHELP_FIND

:HTMLHELP_FIND
set HTMLHELP_HOME=%PF%\HTML Help Workshop
if exist "%HTMLHELP_HOME%" goto BUILD

:HTMLHTLP_NOT_FOUND
echo HTML Help Workshop not found. Build failed!
goto END

:BUILD
echo Using JDK found in %JAVA_HOME%
echo Using DITA found in %DITA_HOME%
echo Using HTML Help Workshop found in %HTMLHELP_HOME%
set ANT_HOME=%DITA_HOME%\tools\ant
set PATH=%JAVA_HOME%\bin;%ANT_HOME%\bin;%HTMLHELP_HOME%;%PATH%
call ant -Ddita.dir="%DITA_HOME%" -f build.xml all
if exist out\help.chm goto END

:HHC_TRY_AGAIN
echo CHM was not build. I try again.
hhc out\help.hhp

:END
rem exit 0
