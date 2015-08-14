<?php

function ctoutvert_settings_form($form, &$form_state) {

    $config = variable_get('ctoutvert_config', array());

    $pdispos_settings = variable_get('prestataires_dispos_settings', array());

    $nodes_types_options = array();

    foreach($pdispos_settings['nodes_types'] as $node_type => $infos) {
        $nodes_types_options[$infos['form_id']] = $node_type;
    }

    $pdispos_constants = _prestataires_dispos_get_constants_by_value();
    $default_dispo_type_options = array();

    foreach($pdispos_constants as $type => $infos) {
        $default_dispo_type_options[$type] = $infos['legend'];
    }

    $last_cron = variable_get('ctoutvert_last_cron', 0);

    if($last_cron > 0) {
        $last_cron_date = DateTime::createFromFormat('U', $last_cron);
        $last_cron_date->setTimezone(new DateTimeZone("Europe/Paris"));
        $last_cron_msg = t("Last update : @date at @hour", array('@date' => $last_cron_date->format('d/m/Y'), '@hour' => $last_cron_date->format('H:i:s')));
    } else {
        $last_cron_msg = "";
    }

    $form = array(

        '#tree' => true,

        'settings' => array(
            '#type' => 'fieldset',
            '#title' => t("Settings"),
            '#collapsible' => true,
            '#collapsed' => true,

            'user' => array(
                '#type' => 'textfield',
                '#title' => t("User"),
                '#default_value' => $config['settings']['user'],
                '#size' => 20,
                '#required' => true,
            ),

            'pwd' => array(
                '#type' => 'textfield',
                '#title' => t("Password"),
                '#default_value' => $config['settings']['pwd'],
                '#size' => 20,
                '#required' => true,
            ),

            'id_engine' => array(
                '#type' => 'textfield',
                '#title' => t("ID engine"),
                '#default_value' => $config['settings']['id_engine'],
                '#size' => 3,
                '#required' => true,
            ),

            'field' => array(
                '#type' => 'textfield',
                '#title' => t("Field Ctoutvert"),
                '#default_value' => $config['settings']['field'],
                '#autocomplete_path' => 'admin/config/ctoutvert/autocomplete/field',
                '#size' => 25,
                '#required' => true,
            ),
        ),

        'cron' => array(
            '#type' => 'fieldset',
            '#title' => "Cron",
            '#collapsible' => true,
            '#collapsed' => true,

            'active' => array(
                '#type' => 'checkbox',
                '#title' => t("Activate cron"),
                '#default_value' => $config['cron']['active'],
            ),

            'time_windows' => array(
                '#type' => 'textfield',
                '#title' => t("Time window(s) for cron"),
                '#description' => t("Two hours (separated by '-') periods, separated by '|'.<br />Exemple : '0-6|13-14' = \"one time between 00h00 (included) and 06h00 (not included) and one time between 13h (included) and 14h (not included)\"."),
                '#default_value' => array_key_exists('cron', $config) ? $config['cron']['time_windows'] : '0-6',
                '#size' => 25,
                '#required' => true,
            ),

            'period_length' => array(
                '#type' => 'textfield',
                '#title' => t("Period length"),
                '#default_value' => array_key_exists('cron', $config) ? $config['cron']['period_length'] : 1,
                '#field_suffix' => t("day(s)"),
                '#size' => 2,
                '#required' => true,
            ),

            'launch' => array(
                '#type' => 'submit',
                '#value' => t("Launch update NOW !"),
                '#submit' => array('ctoutvert_settings_form_cron_submit'),
            ),

            'last_update' => array(
                '#markup' => $last_cron_msg,
            ),
        ),

        'pdispos' => array(
            '#type' => 'fieldset',
            '#title' => t("Prestataires dispos"),
            '#collapsible' => true,
            '#collapsed' => true,
            '#theme' => 'ctoutvert_pdispos_settings',

            'layer' => array(
                '#type' => 'textfield',
                '#title' => t("Dispos layer"),
                '#description' => t("From 1 to 254"),
                '#default_value' => $config['pdispos']['layer'],
                '#required' => true,
                '#size' => 3,
            ),

            'secured_sfid' => array(
                '#type' => 'select',
                '#title' => t("Campings node type"),
                '#options' => $nodes_types_options,
                '#default_value' => $config['pdispos']['secured_sfid'],
            ),

            'default_type' => array(
                '#type' => 'select',
                '#title' => t("Default availabilies type"),
                '#options' => $default_dispo_type_options,
                '#default_value' => $config['pdispos']['default_type'],
            ),

            'availabilities_type' => array(
                '#type' => 'select',
                '#title' => t("Availabilies type to write"),
                '#options' => $default_dispo_type_options,
                '#default_value' => $config['pdispos']['availabilities_type'],
            ),

            'products_mappings' => array(
                '#markup' => t("Fields will appear after first configuration save"),
            ),
        ),

        'save' => array(
            '#type' => 'submit',
            '#value' => t("Save"),
        ),

    );

    if(array_key_exists('pdispos', $config)) {

        $form['pdispos']['products_mappings'] = array();

        $ctoutvert_products = ctoutvert_get_products();

        $sfid = so_forms_get_sfid(null, $config['pdispos']['secured_sfid']);

        $results = db_query(
            "SELECT f.id, d.label FROM {so_forms_fields} AS f
             LEFT JOIN {so_forms_fields_displays} AS d ON d.fid=f.id
             WHERE f.sfid=?
             AND f.field_type!='group'
             ORDER BY f.id ASC",
            array($sfid)
        );

        $pdispos_products_options = array('' => "- " . t("No mapping") . " -");

        while($row = $results->fetchObject()) {
            $pdispos_products_options[$row->id] = $row->label;
        }

        foreach($ctoutvert_products as $type => $label) {
            $form['pdispos']['products_mappings'][$type] = array(
                '#type' => 'select',
                '#title' => $label,
                '#options' => $pdispos_products_options,
                '#default_value' => array_key_exists('products_mappings', $config['pdispos']) ? $config['pdispos']['products_mappings'][$type] : null,
            );
        }
    }

    return $form;
}

function ctoutvert_settings_form_submit($form, &$form_state) {

    $config = array(
        'settings' => $form_state['values']['settings'],
        'cron' => $form_state['values']['cron'],
        'pdispos' => $form_state['values']['pdispos'],
    );

    $config['pdispos']['node_type'] = $form_state['complete form']['pdispos']['secured_sfid']['#options'][$form_state['values']['pdispos']['secured_sfid']];

    variable_set('ctoutvert_config', $config);

    drupal_set_message(t("Configuration has been saved."));
}

function ctoutvert_settings_form_cron_submit($form, &$form_state) {
    ctoutvert_cron(true);
}

function ctoutvert_settings_autocomplete_field($term) {

    $results = db_query("SELECT field_name FROM {field_config} WHERE field_name LIKE ? ORDER BY field_name ASC", array('%' . $term . '%'));

    $fields = array();

    while($row = $results->fetchObject()) {
        $fields[$row->field_name] = $row->field_name;
    }

    drupal_json_output($fields);
}

function theme_ctoutvert_pdispos_settings($vars) {
    $output = "";

    $element = $vars['element'];

    $settings = array(
        '#type' => 'container',
        '#theme_wrappers' => array('fieldset'),
        '#title' => t("Settings"),

        'content' => array(
            '#markup' => render($element['layer']) .
                        render($element['secured_sfid']) .
                        render($element['default_type']) .
                        render($element['availabilities_type']),
        ),
    );

    $output .= render($settings);

    $mappings = array(
        '#type' => 'container',
        '#theme_wrappers' => array('fieldset'),
        '#title' => t("Fields mappings"),

        'content' => array(
            '#markup' => render($element['products_mappings'])
        ),
    );

    $output .= render($mappings);

    return $output;
}