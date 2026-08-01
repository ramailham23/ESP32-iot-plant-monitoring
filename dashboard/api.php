<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');

$host = "localhost";
$user = "root";
$pass = "";
$db = "iot_monitoring";

$conn = new mysqli($host, $user, $pass, $db);

if ($conn->connect_error) {
    http_response_code(500);
    echo json_encode(["error" => $conn->connect_error]);
    exit;
}

$limit = isset($_GET['limit']) ? intval($_GET['limit']) : 20;

$sql = "SELECT id, device_id, suhu, kelembapan, jarak_air, gas_value, waktu 
        FROM sensor_data 
        ORDER BY id DESC 
        LIMIT $limit";

$result = $conn->query($sql);

$data = [];
while ($row = $result->fetch_assoc()) {
    $data[] = $row;
}

$data = array_reverse($data); // urutkan dari lama ke baru buat chart

echo json_encode($data);
$conn->close();
?>
