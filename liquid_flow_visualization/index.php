<?php
/* Database connection settings */
include_once 'db.php';

$data1 = [];
$data2 = [];
$labels = [];

$query = "SELECT sensor_flujo.identificador, sensor_flujo.temperatura, sensor_flujo.flujo, sensor_flujo.fechatiempo FROM sensor_flujo";

$runQuery = mysqli_query($conn, $query);

while ($row = mysqli_fetch_array($runQuery)) {
    $data1[] = $row['temperatura'];
    $data2[] = $row['flujo'];
    $labels[] = '"' . $row['fechatiempo'] . '"';
}

$temperaturas = implode(',', $data1);
$flujos = implode(',', $data2);
$identificadores = implode(',', $labels);
?>

<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <script type="text/javascript" src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.2/Chart.bundle.min.js"></script>
    <title>FLOW LIQUID SENSOR</title>
    <style type="text/css">			
        body {
            font-family: Arial;
            margin: 80px 100px 10px 100px;
            padding: 0;
            color: white;
            text-align: center;
            background: #555652;
        }
        .container {
            color: #E8E9EB;
            background: #222;
            border: #555652 1px solid;
            padding: 10px;
        }
		#lastUpdateTime {
 			color: #999;
    		font-size: 1.0em;
    		margin-top: 10px;
		}
		#team{
			color: #999;
			font-size: 0.9em;
			margin-top: 10px;
		}
    </style>
</head>
<body>	   
    <div class="container">	
        <h1>LIQUID FLOW - SLF3S-4000B</h1>       
        <canvas id="chart" style="width: 100%; height: 65vh; background: #222; border: 1px solid #555652; margin-top: 10px;"></canvas>
        <div id="lastUpdateTime"></div>
		<div id="team">Team: Gisela Espinoza, José Otzoy, Roger Hernández</div>
        <script>
            var ctx = document.getElementById("chart").getContext('2d');
            var myChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: [<?php echo $identificadores; ?>],
                    datasets: [{
                        label: 'Temperature',
                        data: [<?php echo $temperaturas; ?>],
                        backgroundColor: 'transparent',
                        borderColor: 'rgba(255,99,132)',
                        borderWidth: 3
                    },
                    {
                        label: 'Flow',
                        data: [<?php echo $flujos; ?>],
                        backgroundColor: 'transparent',
                        borderColor: 'rgba(0,255,255)',
                        borderWidth: 3	
                    }]
                },
                options: {
                    scales: {
                        yAxes: [{beginAtZero: false}], 
                        xAxes: [{autoskip: true, maxTicketsLimit: 20}]
                    },
                    tooltips: {mode: 'index'},
                    legend: {
                        display: true, 
                        position: 'top', 
                        labels: {
                            fontColor: 'rgb(255,255,255)', 
                            fontSize: 16
                        }
                    }
                }
            });

			
			document.getElementById('lastUpdateTime').innerText = 
    		'Última actualización: ' + new Date().toLocaleTimeString();

			 // Function to update chart
			function updateChart() {
            fetch('update_chart_data.php')
                .then(response => response.json())
                .then(data => {
                    // Update labels
                    myChart.data.labels = data.labels;

                    // Update datasets
                    myChart.data.datasets[0].data = data.temperaturas;
                    myChart.data.datasets[1].data = data.flujos;

                    // Update last update time
                    document.getElementById('lastUpdateTime').innerText = 
                        'Last updated: ' + new Date().toLocaleTimeString();

                    // Render the chart with new data
                    myChart.update();
                })
                .catch(error => {
                    console.error('Error updating chart:', error);
                });
        }

        // Update chart every minute
        setInterval(updateChart, 60000);
        </script>
    </div>
</body>
</html>