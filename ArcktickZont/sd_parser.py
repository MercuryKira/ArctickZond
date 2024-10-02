from peewee import *
from datetime import time, date
import json

with open('output.json', 'r') as json_file:
    data_json = json.load(json_file)
    # print(data_json[0]['g_y'])

conn = SqliteDatabase('samolet.sqlite')


class BaseModel(Model):
    class Meta:
        database = conn


class Message(BaseModel):
    message_id = AutoField(column_name='MessageId')
    count = IntegerField(column_name='count', null=True)
    time = TimeField(column_name='Time', null=True)
    date = DateField(column_name='Data', null=True)
    satellites_count = IntegerField(column_name='Satellites count', null=True)
    pulse_count = IntegerField(column_name='Pulse count', null=True)
    seconds_from_start = IntegerField(column_name='Seconds from start', null=True)
    angel_speed_x = FloatField(column_name='Angel speed x', null=True)
    angel_speed_y = FloatField(column_name='Angel speed y', null=True)
    angel_speed_z = FloatField(column_name='Angel speed z', null=True)
    angel_x = FloatField(column_name='Angel x', null=True)
    angel_y = FloatField(column_name='Angel y', null=True)
    angel_z = FloatField(column_name='Angel z', null=True)
    temp_baro = FloatField(column_name='Temp baro', null=True)
    temp_gyro = FloatField(column_name='Temp gyro', null=True)
    temp_bat = FloatField(column_name='Temp bat', null=True)
    temp_inside = FloatField(column_name='Temp inside', null=True)
    altitude = FloatField(column_name='Altitude', null=True)
    pressure = IntegerField(column_name='Pressure', null=True)
    compass_x = FloatField(column_name='Compass x', null=True)
    compass_y = FloatField(column_name='Compass y', null=True)
    compass_z = FloatField(column_name='Compass z', null=True)
    gps_altitude = FloatField(column_name='GPS Altitude', null=True)
    lat = FloatField(column_name='Latitude', null=True)
    lng = FloatField(column_name='Longitude', null=True)
    gps_speed = FloatField(column_name='GPS speed m/s', null=True)
    gps_course = FloatField(column_name='GPS course', null=True)  # deg
    hdop = FloatField(column_name='HDOP', null=True)
    dose_rate = FloatField(column_name='Dose rate', null=True)

    class Meta:
        table_name = 'Message'


conn.connect()
conn.create_tables([Message])

for data in data_json:
    try:
        date_check = date(data.get('y'), data.get('mh'), data.get('d'))
        time_check = time(data.get('h'), data.get('m'), data.get('s'))
    except Exception as e:
        date_check = None
        time_check = None
        print(e)
    massage = Message(count=data.get('c'),
                      # pulse_count=data.get('p_c'),
                      pulse_count=data.get('particles'),
                      # seconds_from_start=data.get('s_s'),
                      seconds_from_start=data.get('seconds'),
                      angel_speed_x=data.get('g_x'),
                      angel_speed_y=data.get('g_y'),
                      angel_speed_z=data.get('g_z'),
                      angel_x=data.get('a_x'),
                      angel_y=data.get('a_y'),
                      angel_z=data.get('a_z'),
                      temp_baro=data.get('t_b'),
                      temp_gyro=data.get('t_g'),
                      temp_bat=data.get('t_bt'),
                      temp_inside=data.get('t_i'),
                      altitude=data.get('alt'),
                      pressure=data.get('p'),
                      satellites_count=data.get('p'),
                      date=date_check,
                      time=time_check,
                      gps_altitude=data.get('g_alt'),
                      lat=data.get('lat'),
                      lng=data.get('lng'),
                      gps_speed=data.get('mps'),
                      gps_course=data.get('deg'),
                      hdop=data.get('hdop'),
                      compass_x=data.get('c_x'),
                      compass_y=data.get('c_y'),
                      dose_rate=data.get('dose_rate'),
                      compass_z=data.get('c_z'))
    massage.save()

conn.close()
