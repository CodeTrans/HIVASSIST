<?php

  drupal_set_breadcrumb(array(l('Home', '<front>'), l('Tool', 'tool')));
  drupal_set_title('View Regimen Feedback');

  print $results['viewfeedback_table'];
  
  