<div class="testinfo_command-bar">
                
                <div class="testinfo_forms">
                <?php
                    if( !$headless && gz_is_file("$testPath/testinfo.json")
                        && !array_key_exists('published', $test['testinfo'])
                        && ($isOwner || !$test['testinfo']['sensitive'])
                        && (!isset($test['testinfo']['type']) || !strlen($test['testinfo']['type'])) )
                    {
                        $siteKey = GetSetting("recaptcha_site_key", "");
                        if (!isset($uid) && !isset($user) && !isset($USER_EMAIL) && strlen($siteKey)) {
                          echo "<script src=\"https://www.google.com/recaptcha/api.js\" async defer></script>\n";
                          ?>
                          <script>
                          function onRecaptchaSubmit(token) {
                            document.getElementById("urlEntry").submit();
                          }
                          </script>
                          <?php
                        }
                        // load the secret key (if there is one)
                        $secret = GetServerSecret();
                        if (!isset($secret))
                            $secret = '';

                        echo "<form name='urlEntry' id='urlEntry' action='/runtest.php?test=$id' method='POST' enctype='multipart/form-data'>";
                        echo "\n<input type=\"hidden\" name=\"resubmit\" value=\"$id\">\n";
                        echo '<input type="hidden" name="vo" value="' . htmlspecialchars($owner) . "\">\n";
                        if( strlen($secret) ){
                          $hashStr = $secret;
                          $hashStr .= $_SERVER['HTTP_USER_AGENT'];
                          $hashStr .= $owner;

                          $now = gmdate('c');
                          echo "<input type=\"hidden\" name=\"vd\" value=\"$now\">\n";
                          $hashStr .= $now;

                          $hmac = sha1($hashStr);
                          echo "<input type=\"hidden\" name=\"vh\" value=\"$hmac\">\n";
                        }

                        // if TTFB is extra slow (all runs had a ttfb greater than thresold below), then perhaps a site/cdn is purposefully slowing responsse times for bots.
                        // Show a message that offers an option to re-run the tests with a default UA
                        $slowttfbThreshold = 3000;
                        $firstByteTimes = $testResults->getMetricFromRuns("TTFB", false, false );
                        if ( count( $firstByteTimes ) > 0 && min( $firstByteTimes ) > $slowttfbThreshold ) { 
                          echo <<<EOT
                            <details>
                              <summary><strong>High TTFB</strong> <span>Rerun for accuracy...</span></summary> 
                              <div>
                              <p>Some networks intentionally slow performance for bots like the WebPageTest agent. If you suspect this is happening, you can try re-running your test with the browser's original User Agent string to see if it helps.</p>
                              <p><label><input type="checkbox" name="keepua"> Preserve original User Agent string in re-run</label></p>
                            </details>
                          EOT;
                        }


                         echo '<input type="submit" value="Re-run the test">';
                        echo "\n</form>\n";
                    }
                    ?>
                </div>
                <div class="testinfo_artifacts" tabindex="0">
                <h3>Export Files</h3>
                <ul class="testinfo_artifacts-list">
                <?php
                    $fvMedian = $testResults->getMedianRunNumber($median_metric, false);
                    $rvMedian = $testResults->getMedianRunNumber($median_metric, true);

                    echo "<li><a href='/jsonResult.php?test=$id&pretty=1'>View JSON</a></li>";
                    if (is_file("$testPath/test.log"))
                        echo "<li><a href=\"/viewlog.php?test=$id\">View Test Log</a></li>";
                    if (is_file("$testPath/lighthouse.log.gz"))
                        echo "<li><a href=\"/viewlog.php?test=$id&lighthouse=1\">View Lighthouse Log</a></li>";
                    $publish = GetSetting('publishTo');
                    if( $publish && GetSetting('host') != 'www.webpagetest.org' )
                        echo "<li><a href=\"/publish.php?test=$id\">Publish to $publish</a></li>";
                    echo "<li data-artifact-json=\"download\"><a href='/jsonResult.php?test=$id&pretty=1' download>Download JSON</a></li>";
                    echo '<li><a href="/export.php?bodies=1&pretty=1&test=' . $id . '">Download HAR</a></li>';
                    if (is_file("$testPath/test.log"))
                      echo "<li><a href=\"/viewlog.php?test=$id\" download>Download Test Log</a></li>";
                    if (is_file("$testPath/lighthouse.log.gz"))
                      echo "<li><a href=\"/viewlog.php?test=$id&lighthouse=1\" download>Download Lighthouse Log</a></li>";
                ?>
                </ul>
                </div>
            </div>
