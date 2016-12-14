<?php
function hivassist_view_feedback() {
  global $user;
  
  module_load_include('inc', 'webform', '/includes/webform.submissions');

  $node = node_load('48');
  $submissions = webform_get_submissions('48');
  
  $header = array('Details', '', 'Feedback', 'Results', 'Delete');
  
  
//  krumo($submissions);

  foreach ($submissions as $k => $v) {
    (empty($submissions[$k]->data[5][0]) ? $mutations = 'None' : $mutations = implode(', ', $submissions[$k]->data[5]));
    (empty($submissions[$k]->data[6][0]) ? $comorbidities = 'None' : $comorbidities = implode(', ', $submissions[$k]->data[6]));
    (empty($submissions[$k]->data[7][0]) ? $interactions = 'None' : $interactions = implode(', ', $submissions[$k]->data[7]));
    (empty($submissions[$k]->data[13][0]) ? $history = 'None' : $history = implode(', ', $submissions[$k]->data[14]));
    (empty($submissions[$k]->data[10][0]) ? $exclude = 'None' : $exclude = implode(', ', $submissions[$k]->data[10]));

    if (webform_submission_access($node, $v, 'delete', $user)) {
      $delete = l('Delete', "node/$node->nid/submission/$k/delete", array('query' => drupal_get_destination()));
    } else {
      $delete = 'Delete';
    }

    $rows[] = array(
      'Details' => '<strong>Submitted by: </strong>' .$submissions[$k]->name. '<br />
                    <strong>Submitted date: </strong>' .date('m/d/Y', $submissions[$k]->submitted). '<br />
                    <strong>Guideline: </strong>' .$submissions[$k]->data[12][0]. '<br />
                    <strong>Rule: </strong>' .$submissions[$k]->data[2][0]. '<br />
                    <strong>Regimen: </strong>' .$submissions[$k]->data[3][0]. '<br />
                    <strong>Mutations: </strong>' .$mutations. '<br />
                    <strong>Comorbidities: </strong>' .$comorbidities. '<br />
                    <strong>Interactions: </strong>' .$interactions. '<br />
                    <strong>Tropism: </strong>' .$submissions[$k]->data[13][0]. '<br />',
      'More Options' => '<strong>Viral Load: </strong>' .$submissions[$k]->data[8][0]. '<br />
                    <strong>HLA B5701: </strong>' .$submissions[$k]->data[9][0]. '<br />
                    <strong>Poor Adherence: </strong>' .$submissions[$k]->data[11][0]. '<br />
                    <strong>Treatment History: </strong>' .$history. '<br />
                    <strong>Excluded Drugs: </strong>' .$exclude,
      'Feedback' => $submissions[$k]->data[4][0],
      'Results' => l('Results', 'tool/results', array('query' => array(
        'mutations' => $submissions[$k]->data[5][0],
        'comorbidities' => explode(', ', $submissions[$k]->data[6][0]),
        'interactions' => explode(', ', $submissions[$k]->data[7][0]),
        'tropism' => $submissions[$k]->data[13][0],
        'viralload' => $submissions[$k]->data[8][0],
        'hlab5701' => $submissions[$k]->data[9][0],
        'pooradherence' => $submissions[$k]->data[11][0],
        'history' => explode(', ', $submissions[$k]->data[14][0]),
        'exclude' => explode(', ', $submissions[$k]->data[10][0]),
      ))),
      'Delete' => $delete,
    );
  }

  if (empty($rows)) {
    $viewfeedback_table = 'There is no current regimen feedback.';
  } else {
    $per_page = 20;
    $current_page = pager_default_initialize(count($rows), $per_page);
    $chunks = array_chunk($rows, $per_page, TRUE);
    $viewfeedback_table = theme('table', array('header' => $header, 'rows' => $chunks[$current_page]));
    $viewfeedback_table .= theme('pager', array('quantity',count($rows)));          
  }
  
  $results = array(
    'viewfeedback' => $submissions,
    'viewfeedback_table' => $viewfeedback_table,
  );
  
  $output = theme('hivassist_viewfeedback', array('results' => $results));
  return $output;

}
  