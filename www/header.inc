<?php

// Copyright 2020 Catchpoint Systems Inc.
// Use of this source code is governed by the Polyform Shield 1.0.0 license that can be
// found in the LICENSE.md file.
require_once __DIR__ . '/page_data.inc';
require_once __DIR__ . '/include/TestInfo.php';
require_once __DIR__ . '/include/TestResults.php';
require_once __DIR__ . '/include/TestRunResults.php';

if(isset($testPath) ) {
  $testInfo = TestInfo::fromFiles($testPath);
  $testResults = TestResults::fromFiles($testInfo);
}
if (isset($testResults)) {
  $adultKeywords = array();
  if (is_file('./settings/adult.txt'))
    $adultKeywords = file('./settings/adult.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
  $isAdult = $testResults->isAdultSite($adultKeywords);
  if ($isAdult) {
    define('ADULT_SITE', true);
    $adult_site = true;
  }
}

// For users that aren't logged in, include details about the test so it can be stored in indexdb for local history support
if ($id && isset($test) && is_array($test) && 
        isset($test['testinfo']['created']) && 
        isset($test['testinfo']['url']) && 
        !isset($user) && !isset($_COOKIE['google_email'])) {
    $history = array(
        'id' => $id,
        'url' => $test['testinfo']['url'],
        'created' => $test['testinfo']['created'],
        'location' => isset($test['testinfo']['locationText']) ? $test['testinfo']['locationText'] : '',
        'runs' => isset($test['testinfo']['runs']) ? $test['testinfo']['runs'] : 1,
        'label' => isset($test['testinfo']['label']) ? $test['testinfo']['label'] : '',
        'video' => $test['testinfo']['video'] ? true : false
    );
    $history_json = json_encode($history);
    echo "\n<script>\nconst wptTestInfo=$history_json;\n</script>\n";
}

if (!defined('EMBED')) {
?>
<?php
$alert = GetSetting('alert');
if ($alert) {
    echo '<div class="alert-banner">' . $alert . '</div>';
}
?>
<header>
    <a class="logo" href="/"><img src="/images/wpt-logo.svg" alt="WebPageTest, by Catchpoint"/></a>
    <nav>
    <ul id="nav"><?php
        if ($id) {
            $resultUrl = "/results.php?test=$id";
            if (array_key_exists('end', $_REQUEST))
                $resultUrl .= "&end={$_REQUEST['end']}";
            elseif (FRIENDLY_URLS)
                $resultUrl = "/result/$id/";
        }
        if (!isset($navTabs) || !count($navTabs)) {

            if (!GetSetting('disableTestlog')) {
              $navTabs['Test History'] = FRIENDLY_URLS ? '/testlog/1/' : '/testlog.php?days=1';
            }

            if (is_dir('wptmonitor')) {
                $navTabs['Monitor'] = '/wptmonitor/';
            }
            $apiUrl = GetSetting('api_url');
            if ($apiUrl) {
                $navTabs['API'] = $apiUrl;
            }

            if (GetSetting('forums_url')) {
                $navTabs['Forums'] = GetSetting('forums_url');
            }
            $navTabs['Docs'] = 'https://docs.webpagetest.org/';
            $blogUrl = GetSetting('blog_url');
            if ($blogUrl) {
                $navTabs['Blog'] = $blogUrl;
            }

            $navTabs['About'] = FRIENDLY_URLS ? '/about' : '/about.php';

        }
        foreach ($navTabs as $tabName => $tabUrl) {
            // make sure we have a test result to navigate to
            if (strlen($tabUrl)) {
                // highlight the current tab
                $target = '';
                $current = '';
                $opens = '';
                if( !strcasecmp($tabName, $tab) )
                    $current = ' class="current"';
                if (substr($tabUrl, 0, 4) == 'http' && $tabName != 'API') {
                    $target = ' target="_blank" rel="noopener"';
                    $opens = ' (opens in a new tab)';
                }
                if ($opens != '') {
                    echo "<li$current><a title=\"$tabName$opens\" href=\"$tabUrl\"$target>$tabName</a></li>";
                } else {
                    echo "<li$current><a href=\"$tabUrl\"$target>$tabName</a></li>";
                }
                
            }
        }
?></ul>
    </nav>
<?php

if( $supportsAuth && !defined('EMBED') )
{
    echo '<ul id="wptAuthBar">';
    if ($supportsSaml) {
        if (isset($USER_EMAIL) && GetSetting('saml_account')) {
            $saml_account = GetSetting('saml_account');
            $saml_logout = GetSetting('saml_logout');
            if ($saml_logout) {
                $saml_logout = htmlspecialchars($saml_logout);
                $logoutUrl = "javascript:wptLogout('$saml_logout');";
            } else {
                $logoutUrl = "javascript:wptLogout();";
            }
            echo "<li><a href=\"$saml_account\">My Account</a>&nbsp;|&nbsp; <a href=\"$logoutUrl\">Logout</a></li>";
        } else {
            echo '<li><a href="/saml/login.php">Login';
            echo file_get_contents('./images/icon-login.svg');
            echo '</a></li>';
            $register = GetSetting('saml_register');
            if ($register) {
                echo "<li><a class='pill' href='$register' onclick=\"try{if(_gaq!=undefined){_gaq.push(['_trackEvent','Outbound','Click','Signup']);}}catch(err){}\">Sign-up</a></li>";
            }
        }
    } elseif( isset($user) ) {
        $logoutUrl = 'https://www.webpagetest.org/forums/member.php?action=logout';
        echo "<li>Welcome, " . htmlspecialchars($user) . " &nbsp;|&nbsp; <a href=\"$logoutUrl\">Logout</a></li>";
    } else if( isset($_COOKIE['google_email']) && isset($_COOKIE['google_id']) ) {
        $logoutUrl = 'javascript:wptLogout();';
        $google_email = htmlspecialchars($_COOKIE['google_email']);
        echo "<li>Welcome, $google_email &nbsp;|&nbsp; <a href=\"$logoutUrl\">Logout</a></li>";
    } else if (GetSetting('google_oauth_client_id') && GetSetting('google_oauth_client_secret')) {
        echo '<li> &nbsp;|&nbsp; <a href="/oauth/login.php">Login with Google</a></li>';
    }
    echo '</ul>';
}
?>
</header>

<?php
} // EMBED
?>

<div id="main">

<?php


                       

if( isset($tests) ){
    // See if all tests are the same ID or not

    $sameIDs = true;
    foreach($tests as $key=>$value) {
        if($key !== 0 && $value['id'] !== $tests[$key - 1]['id']){
            $sameIDs = false;
        }
    }
    if( $sameIDs ){
        // TODO IF SHARED TEST URL
        $test['test'] = $testInfo->getRawData()['test'];
        $test['testinfo'] =  $testInfo->getRawData()['testinfo'];
    }
}

//If we're looking at a test result, include the extra header section and sub-menu
if( !strcasecmp('Test Result', $tab) && (!isset($nosubheader) || !@$nosubheader) && !defined('EMBED') )
{
    // make sure the test is actually complete
    if(isset($test['test']['completeTime']))
    {
        if (!isset($testResults) || !isset($testInfo)) {
          $testInfo = TestInfo::fromFiles($testPath);
          $testResults = TestResults::fromFiles($testInfo);
        }

        $gradeRun = 1;
        if( array_key_exists('run', $_GET) && $_GET["run"] )
            $gradeRun = intval($_GET["run"]);
        else
        {
            $medianRun = $testResults->getMedianRunNumber($median_metric, false);
            if( $medianRun )
                $gradeRun = $medianRun;
        }
        $gradeRunResults = $testResults->getRunResult($gradeRun, false);
        $firstRunResults = $testResults->getRunResult(1, false);

        echo '<div class="results_header_contain">';
        include ( __DIR__ . '/cta-banner.inc');

        echo '<div id="header_container">';

        
        
        
        
        echo "<div id=\"header_data\"$data_class>";
        // for multistep, link the first URL and show additional text. Otherwise take the test URL
        $numSteps = $gradeRunResults->countSteps();
        $showUrl = $numSteps > 1 || !$url ? $gradeRunResults->getStepResult(1)->getUrl() : $url;
        $shortUrl = str_replace('http://', '',  FitText($showUrl, 180));
        $shortUrl = $numSteps > 1 ? ($shortUrl . " ($numSteps steps)") : $shortUrl;
            echo "<ul class=\"header_data_urltime\"><li><strong>URL: </strong>";
            if (GetSetting('nolinks')) {
                echo "<span class=\"page-tested\">$shortUrl</span>";
            } else {
                echo "<a class=\"url\"  rel=\"nofollow\" title=\"$showUrl\" href=\"$showUrl\">$shortUrl</a>";
            }
            echo "</li>";
            echo "<li><strong>Date: </strong>";
           
            if (isset($test['testinfo']) && (isset($test['testinfo']['completed']) || isset($test['testinfo']['started'])) )
            {
                if (isset($test['testinfo']['completed'])) {
                    $timeStr = $test['testinfo']['completed'];
                } else {
                    $timeStr = $test['testinfo']['started'];
                }
                $completed = (int)$timeStr + ($tz_offset * 60);
                $time = strftime('%x %X', $completed);
                echo "<span class=\"jsdate\" date=\"$timeStr\">$time</span>\n";
            }
            echo "</li></ul>";

            // Note: "Comprehensive" if it's multi device?
            echo "<h1>Webpage Performance Test Result</h1>"; ?>

            <?php
                $flags = array(
                    'ec2-us-east-1' => 'US',
                    'gce-us-west3' => 'US',
                    'ec2-us-west-1' => 'US',
                    'ec2-ca-central-1' => 'CA',
                    'ec2-sa-east-1' => 'BR',
                    'ec2-eu-west-1' => 'IR',
                    'London_EC2_Loc' => 'GB',
                    'ec2-eu-west-3' => 'FR',
                    'gce-europe-west4' => 'NL',
                    'ec2-eu-central-1' => 'DE',
                    'ec2-eu-south-1' => 'IT',
                    'ec2-eu-north-1' => 'SE',
                    'ec2-af-south-1' => 'ZA',
                    'ec2-me-south-1' => 'BS',
                    'azure-uae-north' => 'AE',
                    'ap-south-1' => 'IN',
                    'tencent-bangkok' => 'TH',         
                    'ec2-ap-southeast-1' => 'SG',      
                    'gce-asia-southeast2' => 'MC',     
                    'ec2-ap-east-1' => 'HK',           
                    'tencent-shanghai' => 'CN',        
                    'ec2-cn-north-1' => 'CN',          
                    'tencent-beijing' => 'CN',         
                    'ec2-cn-northwest-1' =>  'CN',     
                    'ec2-ap-northeast-2' =>  'KR',     
                    'ec2-ap-northeast-1' => 'JP',      
                    'ec2-ap-northeast-3' => 'JP',      
                    'azure-australia-southeast' => 'AU',
                    'ec2-ap-southeast-2' => 'AU',
                    'LosAngeles_M1MacMini' => 'US',
                    'NewYork_M1MacMini' => 'US',
                    'LosAngeles_M1MacMini' => 'US',
                    'NYC_Android' => 'US',
                    'Dulles_Android' => 'US'
                );

            $locationSimple = explode( "-", $test['testinfo']['locationLabel'] )[ 0 ];
            $connectivity = $test['test']['connectivity'];
            $browser = $test['testinfo']['browser'];
            //check to make sure we have a browser icon
            if (file_exists(__DIR__.'/images/test_icons/' . strtolower($browser) . '.svg')) {
                //exists so let's use it
                $browserIcon = true;
            } else {
                $browserIcon = false;
            }
            if ($test['testinfo']['mobile'] == 1) {
                $device = strtolower( $test['testinfo']['mobileDevice'] );
            } else {
                $device = "Desktop";
            }

            ?>

            <div class="test_presets">
                <ul>
                    <?php 
                    echo '<li><strong>'.$device.'</strong>';
                    if ($browserIcon) {
                        echo '<img src="/images/test_icons/'. strtolower($browser) . '.svg" alt="'. $browser . '">';
                    } else {
                        echo '<span class="test_presets_tag">'.$browser.'</span>';
                    }
                    
                    echo '<span class="test_presets_tag">'. $connectivity . '</span>';

                    echo '<span class="test_presets_tag">';

                    if( $flags[$test['test']['loc']] ){
                        echo '<img src="/images/test_icons/flags/' . $flags[$test["test"]["loc"]] . '.svg" alt="' . $locationSimple . '" title="'. $locationSimple .'">';
                    }
                    
                    echo $locationSimple;
                    echo '</span>';
                    // TODO what is show sensitive?
                    //$show_sensitive = false;
                    echo '<details class="test_presets_more"><summary><span class="test_presets_tag">More</span></summary><div class="details_contain">';
                    echo GetTestInfoHtml();
                    echo '</div></details>';
                    
                    ?>
                    </li>
                    
                </ul>
            </div>



    <?php
    
             echo "<details class=\"heading_details\"><summary>More info</summary><p>";
            
            if( $dom )
                echo 'DOM Element: <b>' . $dom . '</b><br>';
            if( array_key_exists('authenticated', $test['test']) && (int)$test['test']['authenticated'] == 1)
                echo '<b>Authenticated: ' . $login . '</b><br>';
            if( (int)$test['test']['connections'] !== 0)
                 echo '<b>' . $test['test']['connections'] . ' Browser connections</b><br>';
            if( array_key_exists('script', $test['test']) && strlen($test['test']['script']) )
                echo '<b>Scripted test</b><br>';
            if( strlen($blockString) )
                echo "Blocked: <b>$blockString</b><br>";
            if (isset($test['testinfo']['context']) && strlen($test['testinfo']['context']))
            {
                echo 'Context: ';
                $contextText = htmlspecialchars($test['testinfo']['context']);
                if (isset($test['testinfo']['context_url']) && strlen($test['testinfo']['context_url'])) {
                    $contextUrl = $test['testinfo']['context_url'];
                    if (GetSetting('nolinks')) {
                        echo "<span class=\"medium colored\">$contextText</span>";
                    } else {
                        echo "<a class=\"url\" rel=\"nofollow\" title=\"$contextUrl\" href=\"$contextUrl\">$contextText</a>";
                    }
                } else {
                    echo $contextText;
                }
                echo '<br>';
            }
            if ( $test['testinfo']['label'] ) {
                echo '<strong>Test Label: </strong>' . $test['testinfo']['label']  . '</strong>';
              }

        echo '</p></details></div>';
        echo '<div class="header_screenshots">';
        
        require_once __DIR__ . '/screen_shot_embed.php';
        echo '</div></div>';


        
        echo '</div>';

?>

       
      <?php 
        if( !isset($hideResultsNav) ){
        ?>
        <div class="results_nav_contain">
        <div class="results_nav">

              <p class="results_nav_hed">View:</p>
              <nav class="link_select" tabindex="0" aria-label="Test Type">
        <span class="link_select_label"><?php echo $subtab; ?></span>
        <span class="visually-hidden"> or...</span>
        

        <?php 
            if( !$id && $test['id'] && isset($tests) ){
                $id = $test['id'];
            }
            echo '<ul>';
        if( !isset($run) )
            $run = $gradeRun;
        $useFriendlyUrls = !isset($_REQUEST['end']) && FRIENDLY_URLS;
        $menuUrlGenerator = UrlGenerator::create($useFriendlyUrls, "", $id, $run, !empty($cached));
        $endParams = isset($_REQUEST['end']) ? ("end=" . $_REQUEST['end']) : "";

        $tabs = array( 'Performance Summary' => $menuUrlGenerator->resultSummary($endParams));

        $tabs['Filmstrip'] = $menuUrlGenerator->filmstripView("filmstrip", $endParams);

        $tabs['Details'] = $menuUrlGenerator->resultPage("details", $endParams);

        $gradedRunResults = $testResults->getRunResult($gradeRun, !empty($cached));



        if ($gradedRunResults->hasWebVitals())
            $tabs['Web Vitals'] = $menuUrlGenerator->resultPagePHP("vitals", $endParams);


        if ($gradedRunResults->isOptimizationChecked())
            $tabs['Performance'] = $menuUrlGenerator->resultPage("performance_optimization", $endParams);

        $tabs['Content'] = $menuUrlGenerator->resultPage("breakdown", $endParams);
        $tabs['Domains'] = $menuUrlGenerator->resultPage("domains", $endParams);
        if ($gradedRunResults->hasBreakdownTimeline()) {
          // currently only supported by standard urls
          $menuStandardUrlGenerator = UrlGenerator::create(false, "", $id, $run, !empty($cached));
          $tabs['Processing'] = $menuStandardUrlGenerator->resultPage("breakdownTimeline");
        }
        

      
        foreach( $tabs as $tabName => $tabUrl )
        {
            // make sure we have a test result to navigate to
            if( strlen($tabUrl) )
            {
                // highlight the current tab
                $current = '';
                if( !strcasecmp($tabName, $subtab) )
                    $current = ' class="current"';
                echo "<li$current><a href=\"$tabUrl\">$tabName</a></li>";
            }
        }

        // Add the "Image Analysis" link
        if (isset($id) && strlen($id)) {
          $tabUrl = GetSetting('image_analysis');
          if ($tabUrl) {
            $tabUrl = str_replace('{test id}', $id, $tabUrl);
            $onclick = "try{if(_gaq!=undefined){_gaq.push(['_trackEvent','Outbound','Click','Cloudinary']);}}catch(err){}";
            echo "<li><a href=\"$tabUrl\" onclick=\"$onclick\" title=\"Cloudinary Image Analysis (external) - Opens in a new window.\" target=\"_blank\" rel=\"noopener\" >Image Analysis<img src=\"/images/icon-external.svg\" alt=\"\"></a></li>";
          }
        }

        // Add the "Request Map" link
        if (isset($id) && strlen($id)) {
          $tabUrl = GetSetting('request_map');
          if ($tabUrl) {
            $tabUrl = str_replace('{test id}', $id, $tabUrl);
            $onclick = "try{if(_gaq!=undefined){_gaq.push(['_trackEvent','Outbound','Click','RequestMap']);}}catch(err){}";
            echo "<li><a href=\"$tabUrl\" onclick=\"$onclick\" title=\"Request Map (external) - Opens in a new window.\" target=\"_blank\" rel=\"noopener\" >Request Map<img src=\"/images/icon-external.svg\" alt=\"\"></a></li>";
          }
        }

        // one dollar sign for every 500KB
        $dollars = "";
        $dollarcount = max(1, min(5, ceil($gradeRunResults->getStepResult(1)->getMetric("bytesIn") / (500 * 1024))));
        for ($i = 1; $i <= 5; $i++)
        $dollars .= $i <= $dollarcount ? '$' : '-';
        if(!$id){
        $id = $testInfo->getId();
        }
        $onclick = "try{if(_gaq!=undefined){_gaq.push(['_trackEvent','Outbound','Click','WhatDoesMySiteCost']);}}catch(err){}";
        echo "<li><a title=\"Find out how much it costs for someone to use your site on mobile networks around the world. (external) - Opens in a new window.\" target=\"_blank\" rel=\"noopener\" " .
        "href=\"http://whatdoesmysitecost.com/index.php?testID=$id\" onclick=\"$onclick\">Data Cost: $dollars <img src=\"/images/icon-external.svg\" alt=\"\"></a></li>";

    

        echo '</ul></span></div></div>';

        }
        ?>





        

<?php
        echo '<div id="test_results-container">';

        echo '<div id="test-1" class="test_results">';
        
        echo '<div class="test_results-content">';
    }
    else
    {
    ?>
        <div id="test_results_running-container">
            <div id="test-1" class="test_results">
                <div class="test_results-content">
    <?php
    }
}

// include the comparison UI pop-up dialog
/*
if( !strcasecmp('Test Result', $tab) || (array_key_exists('compare', $_COOKIE) && strlen($_COOKIE['compare']))) {
    echo '<div id="compare-dlg">';
    echo 'Compare';
    echo '</div>';
}
*/




?>
<script type="text/javascript">
  markUserTime('aft.Site Header');
</script>
