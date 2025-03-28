<?php
/* Database connection settings */
include_once 'db.php';

header('Content-Type: application/json');

$data1 = [];
$data2 = [];
$labels = [];

$query = "SELECT sensor_flujo.identificador, sensor_flujo.temperatura, sensor_flujo.flujo, sensor_flujo.fechatiempo FROM sensor_flujo";

$runQuery = mysqli_query($conn, $query);

while ($row = mysqli_fetch_array($runQuery)) {
    $data1[] = $row['temperatura'];
    $data2[] = $row['flujo'];
    $labels[] = $row['fechatiempo'];
}

echo json_encode([
    'temperaturas' => $data1,
    'flujos' => $data2,
    'labels' => $labels
]);
?>